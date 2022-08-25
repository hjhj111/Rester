#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H
#pragma once
#include<unordered_map>
#include<regex>
#include<vector>

#include "../utls.h"

//using namespace std;

//for http header checking
inline bool StrSame(const std::string& s1, const std::string& s2)
{
    if(s1.size()!=s2.size())
    {
        return false;
    }
    for(int i=0;i<s1.size();i++)
    {
        if(s1[i]!=s2[i]&&std::abs(s1[i]-s2[i])!=26)
        {
            return false;
        }
    }
    return true;
}

class Response
{
public:
    Response()
        : head_len_(0),
          protocol_("HTTP/1.1"),
          major_version_(1),
          minor_version_(1),

          response_(nullptr),
          response_len_(0),
          data_size_(0),
          status_code_(0),
          data_(nullptr)
    {

    }

    ~Response()
    {
        if(response_ != nullptr)
        {
            delete response_;
            response_= nullptr;
        }
        if(data_ != nullptr)
        {
            delete data_;
            data_= nullptr;
        }
        printf("response delete\n");
//        delete m_response;
//        m_response= nullptr;
//        delete m_data;
//        m_data= nullptr;
    }

    void CombineResponse()
    {
        head_.clear();
        head_.append(protocol_);
        head_.append(" ");
        head_.append(to_string(status_code_));
        head_.append(" ");
        head_.append(status_description_);
        head_.append("\r\n");

        bool has_content_length=false;
        for (auto header : headers_)
        {
            if(StrSame(header.first, "Content-Length"))
            {
                has_content_length=true;
            }
            head_.append(header.first);
            head_.append(": ");
            head_.append(header.second);
            head_.append("\r\n");
        }
        if(!has_content_length)
        {
            head_.append("Content-Length");
            head_.append(": ");
            head_.append(to_string(data_size_));
            head_.append("\r\n");
        }
        head_.append("\r\n");
        //m_head.append(m_data);
        head_len_ = head_.size();
        response_len_= head_len_ + data_size_;
        response_=new char[response_len_ + 1000];
        memcpy(response_, head_.c_str(), head_len_);
        memcpy(response_ + head_len_, data_, data_size_);
    }

    std::string BodyStr()
    {
        return data_;
    }

    const char* BodyData()
    {
        return data_;
    }

    void SetData(const char* data, int datasize)
    {
        if(data_ != nullptr)
        {
            delete data_;
        }
        data_=new char[datasize];
        memcpy(data_, data, datasize);
        data_size_=datasize;
    }

    int DataSize()
    {
        return data_size_;
    }

    std::string Protocol()
    {
        return protocol_;
    }

    void Protocol(std::string protocol)
    {
        protocol_ = protocol;
    }

    pair<int, int> Version()
    {
        return {major_version_, minor_version_ };
    }

    const std::string& HeadersStr()
    {
        return head_;
    }

    int HeadersLen()
    {
        return head_len_;
    }

    char* ResponseData()
    {
        return response_;
    }

    int ResponseLen()
    {
        return response_len_;
    }

    std::string StatusDescription()
    {
        return status_description_;
    }

    void SetStatusDescription(std::string statusDescription)
    {
        status_description_ = statusDescription;
    }

    int StatusCode()
    {
        return status_code_;
    }

    void SetStatusCode(int statusCode)
    {
        status_code_ = statusCode;
    }

    //if name was not set, return ""
    std::string FindHeader(const std::string& name)
    {
        if (headers_.find(name) != headers_.end())
        {
            return headers_.at(name);
        }
        return "";
    }

    // return 1 if name exist, or return 0
    int SetHeader(const std::string& name, const std::string& value)
    {
        if (headers_.find(name) != headers_.end())
        {
            headers_[name] = value;
            return 1;
        }
        else
        {
            headers_.insert(make_pair(name, value));
            return 0;
        }
    }

    //for test origin bytes
    void PrintResponse()
    {
        for (int i=0; i < head_.size(); i++)
        {
            auto ch = head_[i];
            if (ch == '\r')
            {
                printf("\\r\\n\n");
            }
            else if (ch == '\n')
            {

            }
            else
            {
                printf("%c", ch);
            }
        }
        printf("\n");
    }

private:
    char* response_;
    int response_len_;
    std::string head_;
    int head_len_;
    std::string protocol_;
    unsigned int major_version_;
    unsigned int minor_version_;
    int status_code_;
    std::string status_description_;
    unordered_map<std::string, std::string> headers_;
    char* data_;
    int data_size_;
};

using ResponsePtr=shared_ptr<Response>;

#endif // HTTPRESPONSE_H
