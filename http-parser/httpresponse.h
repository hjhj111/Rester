#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H
#pragma once
#include<string>
#include<unordered_map>
#include<regex>
#include<vector>

using namespace std;


class Response
{
public:
    Response()
        : m_head_len(0),
          m_Protocol("HTTP/1.1"),
          m_MajorVersion(1),
          m_MinorVersion(1)
    {

    }

    ~Response()
    {
        delete m_response;
        delete m_data;
    }

    void combineResponse()
    {
        m_head.clear();
        m_head.append(m_Protocol);
        m_head.append(" ");
        m_head.append(to_string(m_StatusCode));
        m_head.append(" ");
        m_head.append(m_StatusDescription);
        m_head.append("\r\n");

        for (auto header : m_Headers)
        {
            m_head.append(header.first);
            m_head.append(": ");
            m_head.append(header.second);
            m_head.append("\r\n");
        }
        m_head.append("\r\n");
        //m_head.append(m_data);
        m_head_len = m_head.size();
        m_response_len= m_head_len + m_DataSize;
        m_response=new char[m_response_len+1000];
        memcpy(m_response, m_head.c_str(), m_head_len);
        memcpy(m_response + m_head_len, m_data, m_DataSize);
    }

    string getData()
    {
        return m_data;
    }

    void setData(const char* data, int datasize)
    {
        //m_data.assign(data, datasize);
        m_data=new char[datasize];
        memcpy(m_data,data,datasize);
        m_DataSize=datasize;
    }

    int getDataSize()
    {
        return m_DataSize;
    }

    string getProtocol()
    {
        return m_Protocol;
    }

    void setProtocol(string protocol)
    {
        m_Protocol = protocol;
    }

    pair<int, int> getVersion()
    {
        return { m_MajorVersion,m_MinorVersion };
    }

    const string getHead()
    {
        return m_head;
    }

    int getHeadLen()
    {
        return m_head_len;
    }

    char* getResponse()
    {
        return m_response;
    }

    int getResponseLen()
    {
        return m_response_len;
    }

    string getStatusDescription()
    {
        return m_StatusDescription;
    }

    void setStatusDescription(string statusDescription)
    {
        m_StatusDescription = statusDescription;
    }

    int getStatusCode()
    {
        return m_StatusCode;
    }

    void setStatusCode(int statusCode)
    {
        m_StatusCode = statusCode;
    }

    string findHeader(string name)
    {
        if (m_Headers.find(name) != m_Headers.end())
        {
            return m_Headers.at(name);
        }
        return "";
    }

    int setHeader(string name, string value)
    {
        if (m_Headers.find(name) != m_Headers.end())
        {
            m_Headers[name] = value;
            return 1;
        }
        else
        {
            m_Headers.insert(make_pair(name, value));
            return 0;
        }
    }

    void printResponse()
    {
        for (int i=0; i < m_head.size(); i++)
        {
            auto ch = m_head[i];
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


    //file
    //char* buf_;
    //int buf_size_=0;//must be initialized
    char* m_response;
    int m_response_len;

private:
    string m_head;
    int m_head_len;
    string m_Protocol;
    unsigned int m_MajorVersion;
    unsigned int m_MinorVersion;
    int m_StatusCode;
    string m_StatusDescription;
    unordered_map<string, string> m_Headers;
    char* m_data;
    int m_DataSize;


};

using ResponsePtr=shared_ptr<Response>;

#endif // HTTPRESPONSE_H
