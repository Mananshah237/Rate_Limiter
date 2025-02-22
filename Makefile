all: rate_limiter lambda_handler

rate_limiter: rate_limiter.cpp
	g++ -std=c++17 rate_limiter.cpp -o rate_limiter -lhiredis -pthread

lambda_handler: lambda_handler.cpp
	g++ -std=c++17 lambda_handler.cpp -o lambda_handler -laws_lambda -lhiredis -pthread
	zip lambda.zip lambda_handler

run: rate_limiter
	./rate_limiter

deploy: lambda_handler
	aws lambda update-function-code --function-name RateLimiter --zip-file fileb://lambda.zip

clean:
	rm -f rate_limiter lambda_handler lambda.zip
