#ifndef __HTTPUTILS_H__
#define __HTTPUTILS_H__

#include "cpprest/http_listener.h"

void SafeReply(const web::http::http_request &request,
               web::http::status_code status);
void SafeReply(const web::http::http_request &request,
               web::http::status_code status, const utf8string &body_data);
void SafeReply(const web::http::http_request &request,
               web::http::status_code status,
               const concurrency::streams::istream &body_data,
               const utility::string_t &content_type);

void HandleError(pplx::task<void> &t);

utility::string_t GetSupportedMethods();

void HandleUnSupportMethod(const web::http::http_request &request);

void HandleHealthCheck(const web::http::http_request &request);

class HttpRequestLimiter {
  public:
   HttpRequestLimiter(HttpRequestLimiter &&) = delete;
   HttpRequestLimiter &operator=(HttpRequestLimiter &&) = delete;
   HttpRequestLimiter(const HttpRequestLimiter &) = delete;
   HttpRequestLimiter &operator=(const HttpRequestLimiter &) = delete;

   HttpRequestLimiter();
   virtual ~HttpRequestLimiter();

   static std::shared_ptr<HttpRequestLimiter> GetInstance();
   static uint64_t max_request_;
   static std::atomic_size_t request_count_;

  private:
   static std::mutex request_mutex_;
};

#endif /* __HTTPUTILS_H__ */