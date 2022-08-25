#pragma once
#include <string>
#include <map>
#include <sstream>
#include <regex>

class HttpParser{
private:
    std::map<std::string, std::string> http_;
    std::string format_key(std::string &str);
public:
    HttpParser(char *buf);
    ~HttpParser()
    {

    }
    void Show();
    std::string operator[](std::string str);

    std::string GetUrlParameter(string key)
    {
        return GetParameter(key, http_.at("url_paras"));
    }

private:
    std::string GetParameter(string key, string url)
    {
        smatch result;
        if (std::regex_search(url.cbegin(), url.cend(), result, std::regex(key + "=(.*?)&")))
        {
            return result[1];
        }
        else if (regex_search(url.cbegin(), url.cend(), result, regex(key + "=(.*)")))
        {
            return result[1];
        }
        else
        {
            return "";
        }
    }
};

using RequestPtr=shared_ptr<HttpParser>;

inline HttpParser::HttpParser(char* msg){
    std::string buf(msg);
    std::istringstream buf_stream(buf);
    enum parts{
        start_line,
        headers,
        body
    };
    parts part = start_line;
    std::string line;
    std::string body_string;
    while(getline(buf_stream, line)){
        switch (part)
        {
        case start_line:
        {
            std::istringstream line_stream(line);
            std::string tmp;
            line_stream >> tmp;
            if(tmp.find("HTTP") == std::string::npos){
                http_.insert(std::make_pair("method", tmp));
                line_stream >> tmp;
                http_.insert(std::make_pair("url_paras", tmp));
                //find url
                string url;
                smatch result;
                if (std::regex_search(tmp.cbegin(), tmp.cend(), result, std::regex("/.*?/\?")))
                {
                     url=result[0];
                     //getchar();
                }
                else
                {
                    url=tmp;
                }
                http_.insert(std::make_pair("url", url));

                line_stream >> tmp;
                http_.insert(std::make_pair("version", tmp));
            } else{
                http_.insert(std::make_pair("version", tmp));
                line_stream >> tmp;
                http_.insert(std::make_pair("status", tmp));
                line_stream >> tmp;
                http_.insert(std::make_pair("status_text", tmp));
            }
            part = headers;
            break;
        }
        case headers:
        {
            if(line.size() == 1){
                part = body;
                break;
            }
            auto pos = line.find(":");
            if(pos == std::string::npos)
                continue;
            std::string tmp1(line, 0, pos);
            std::string tmp2(line, pos + 2);
            http_.insert(std::make_pair(format_key(tmp1), tmp2));
            break;
        }
        case body:
        {
            body_string.append(line);
            body_string.push_back('\n');
            break;
        }
        default:
            break;
        }
    }
    http_.insert(std::make_pair("body", body_string));
}

//HttpParser::~HttpParser(){}

inline void HttpParser::Show(){
    printf("Request parser result:-----------------\n");
    for(auto it = http_.cbegin(); it != http_.cend(); ++it){
        //std::cout << it->first << ": " << it->second << std::endl;
        printf("%s: %s\n",it->first.c_str(),it->second.c_str());
    }
    printf("Request parser result:+++++++++++++++++\n");

}

inline std::string HttpParser::operator[](std::string str){
    //auto it = http.find(format_key(str));
    auto it = http_.find(str);
    return it != http_.end() ? it->second : "";
}

inline std::string HttpParser::format_key(std::string &str){
    if(str[0] >= 'a' && str[0] <= 'z'){
        str[0] = str[0] + 'A' - 'a';
    }
    int position = 0;
    while((position = str.find("-", position)) != std::string::npos){
        if(str[position + 1] >= 'a' && str[position + 1] <= 'z'){
            str[position + 1] = str[position + 1] + 'A' - 'a';
        }
        position++;
    }
    return str;
}

