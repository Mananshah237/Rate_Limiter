#include <aws/lambda-runtime/runtime.h>
#include <aws/core/Aws.h>
#include <hiredis/hiredis.h>
#include <string>

using namespace aws::lambda_runtime;

RateLimiter limiter(10, 5.0, "your-redis-endpoint", 6379); // Replace with Redis endpoint

invocation_response handler(invocation_request const& req) {
    if (limiter.allow_request()) {
        return invocation_response::success("Request allowed", "text/plain");
    }
    return invocation_response::failure("Rate limit exceeded", "text/plain");
}

int main() {
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    run_handler(handler);
    Aws::ShutdownAPI(options);
    return 0;
}
