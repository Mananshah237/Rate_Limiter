#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <hiredis/hiredis.h> // Redis C client

class RateLimiter {
private:
    const int capacity_;         // Max tokens
    const double refill_rate_;   // Tokens per second
    double tokens_;              // Current tokens
    std::mutex mtx_;             // Thread safety
    redisContext* redis_;        // Redis connection

    void refill() {
        auto now = std::chrono::steady_clock::now();
        static auto last_refill = now;
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - last_refill).count() / 1e6;
        tokens_ = std::min(static_cast<double>(capacity_), tokens_ + elapsed * refill_rate_);
        last_refill = now;

        // Sync with Redis
        redisCommand(redis_, "SET tokens %f", tokens_);
    }

public:
    RateLimiter(int capacity, double refill_rate, const char* redis_host, int redis_port)
        : capacity_(capacity), refill_rate_(refill_rate), tokens_(capacity) {
        redis_ = redisConnect(redis_host, redis_port);
        if (redis_ == nullptr || redis_->err) {
            std::cerr << "Redis connection failed" << std::endl;
            exit(1);
        }
        // Load initial tokens from Redis
        redisReply* reply = (redisReply*)redisCommand(redis_, "GET tokens");
        if (reply && reply->type == REDIS_REPLY_STRING) {
            tokens_ = std::stod(reply->str);
        }
        freeReplyObject(reply);
    }

    ~RateLimiter() { redisFree(redis_); }

    bool allow_request() {
        std::lock_guard<std::mutex> lock(mtx_);
        refill();
        if (tokens_ >= 1.0) {
            tokens_ -= 1.0;
            redisCommand(redis_, "SET tokens %f", tokens_);
            return true;
        }
        return false;
    }
};

void simulate_requests(RateLimiter& limiter, int id) {
    for (int i = 0; i < 5; ++i) {
        if (limiter.allow_request()) {
            std::cout << "Thread " << id << ": Request allowed" << std::endl;
        } else {
            std::cout << "Thread " << id << ": Request denied" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

int main() {
    RateLimiter limiter(10, 5.0, "localhost", 6379); // 10 tokens, 5 tokens/sec, local Redis
    std::thread t1(simulate_requests, std::ref(limiter), 1);
    std::thread t2(simulate_requests, std::ref(limiter), 2);
    t1.join();
    t2.join();
    return 0;
}
