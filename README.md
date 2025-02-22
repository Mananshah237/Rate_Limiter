# Distributed Rate Limiter for Cloud Services
A C++17-based rate limiter using a token-bucket algorithm, integrated with Redis for distributed quota tracking and AWS Lambda for cloud deployment.

## Features
- Multi-threaded C++17 implementation
- Real-time quota sync with Redis
- Scalable AWS Lambda deployment

## Setup
1. Install dependencies: `libhiredis-dev`, `aws-sdk-cpp`
2. Run locally: `make run`
3. Deploy to AWS: `make deploy`

## Usage
- Local: `./rate_limiter`
- Lambda: Trigger via API Gateway
