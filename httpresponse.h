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
        :m_len(0),
        m_Protocol("HTTP/1.1"),
        m_MajorVersion(1),
        m_MinorVersion(1)
    {

    }

    ~Response()
    {

    }

    void combineRsponse()
    {
        m_buf.clear();
        m_buf.append(m_Protocol);
        m_buf.append(" ");
        m_buf.append(to_string(m_StatusCode));
        m_buf.append(" ");
        m_buf.append(m_StatusDescription);
        m_buf.append("\r\n");

        for (auto header : m_Headers)
        {
            m_buf.append(header.first);
            m_buf.append(": ");
            m_buf.append(header.second);
            m_buf.append("\r\n");
        }
        m_buf.append("\r\n");
        m_buf.append(m_data);
        m_len = m_buf.size();
    }

    string getData()
    {
        return m_data;
    }

    void setData(const char* data, int datasize)
    {
        m_data.assign(data, datasize);
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

    const string getBuf()
    {
        return m_buf;
    }

    int getLen()
    {
        return m_len;
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
        for (int i=0;i<m_buf.size();i++)
        {
            auto ch = m_buf[i];
            if (ch == '\r')
            {
                printf("\\r\\n%s", "\r\n");
            }
            if (ch == '\n')
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
    string m_buf;
    int m_len;
    string m_Protocol;
    unsigned int m_MajorVersion;
    unsigned int m_MinorVersion;
    int m_StatusCode;
    string m_StatusDescription;
    unordered_map<string, string> m_Headers;
    string m_data;
    int m_DataSize;
};



#endif // HTTPRESPONSE_H
