#if 0
#include "speech.h"
#include "nlp.h"
#include <iostream>

std::string app_id = "10613893";
std::string api_key = "XWzWZV7Q8gWpNyIjyMZ5qbAG";
std::string secret_key = "uVu1S5KKfp5lxxmzjY9psXDBR2iOG2Mm";

std::string app_nlpid = "10615361";
std::string api_nlpkey = "w4nfGmqG8lgOzgLCwvj4Fq8T";
std::string secret_nlpkey = "VY0A4DVKLKLPtVP14Cc96HZehB26PelU";

aip::Speech client(app_id, api_key, secret_key);
aip::Nlp nlpclient(app_nlpid, api_nlpkey, secret_nlpkey);

//// 识别远程文件
//void asr(aip::Speech client)
//{
//    // 无可选参数调用接口
//    Json::Value result =
//    client.recognize_url("http://bos.nj.bpc.baidu.com/v1/audio/8k.amr",
//                "http://your_site/dump",
//                "amr", 8000, aip::null);
//    std::cout << result << std::endl;

//    // 如果需要覆盖或者加入参数
//    std::map<std::string, std::string> options;
//    options["lan"] = "en";
//    result = client.recognize_url("http://bos.nj.bpc.baidu.com/v1/audio/8k.amr",
//                "http://your_site/dump",
//                "amr", 8000, options);
//    std::cout << result << std::endl;

//}

// 识别本地文件
void asr(aip::Speech client)
{
    // 无可选参数调用接口
    std::string file_content;
    aip::get_file_content("/home/pengjialing/qttest/hello.wav", &file_content);
    Json::Value result = client.recognize(file_content, "wav", 16000, aip::null);
    std::cout << result << std::endl;

    // 如果需要覆盖或者加入参数
    std::map<std::string, std::string> options;
    options["lan"] = "en";
    result = client.recognize(file_content, "wav", 16000, options);
    std::cout << result << std::endl;
}

int main()
{
//    asr(client);
    std::map<std::string, std::string> options;
    options["lan"] = "en";
//    Json::Value result = nlpclient.lexer_custom("我要去我的世界", aip::null);
    asr(client);
//    Json::FastWriter fastWriter;
//    std::string output = fastWriter.write(result);
//    std::cout << result << std::endl;
}
#endif
