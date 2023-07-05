#include <iostream>
#include <string>
#include <curl/curl.h>
#include <vector>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <chrono>
#include <thread>

//Credentials
std::string clientId = "YOUR_CLIENT_ID"; //change here
std::string clientSecret = "YOUR_CLIENT_SECRET"; //change here
std::string email = "YOUR_EMAIL"; //change here
std::string password = "YOUR_PASSWORD"; //change here


//METABYPASS TOKEN FILE PATH
std::string tokenFilePath = "metabypass.token";

// ------------------------ HELPER FUNCTIONS -----------------------------
bool fileExists(const std::string& filePath) {
    std::ifstream file(filePath);
    return file.good();
}


std::string getFileContents(const std::string& filePath) {
    std::ifstream file(filePath);
    if (file.is_open()) {
        std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        return contents;
    }
    return "-1";
}


bool writeFile(const std::string& path, const std::string& data) {
    std::ofstream file(path);
    if (file.is_open()) {
        file << data;
        file.close();
        return true;
    } else {
        return false;
    }
}


std::string base64_encode(const std::vector<unsigned char>& data) {

    const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string encoded;
    int padding = (3 - data.size() % 3) % 3;

    for (size_t i = 0; i < data.size(); i += 3) {
        unsigned int triple = (data[i] << 16) + (data[i + 1] << 8) + data[i + 2];

        for (int j = 0; j < 4; j++) {
            int index = (triple >> (6 * (3 - j))) & 0x3F;
            encoded += base64_chars[index];
        }
    }

    encoded.insert(encoded.end(), padding, '=');
    return encoded;
}


std::string convert_image_to_base64(const std::string& image_path) {
    std::ifstream file(image_path, std::ios::binary);
    if (file) {
        std::vector<unsigned char> data(std::istreambuf_iterator<char>(file), {});
        file.close();
        return base64_encode(data);
    }
    return "-3";
}


// Callback function to write response data into a string
size_t WriteCallback(char* contents, size_t size, size_t nmemb, std::string* response) {
    size_t totalSize = size * nmemb;
    response->append(contents, totalSize);
    return totalSize;
}

// Function to send a POST request with headers and a body
std::string sendPostRequest(const std::string& url, const std::vector<std::string>& headers, const std::string& body) {
    CURL* curl = curl_easy_init();
    std::string response;

    if (curl) {
        // Set the URL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // Set the request type to POST
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        // Set the headers
        curl_slist* headerList = NULL;
        for (const std::string& header : headers) {
            headerList = curl_slist_append(headerList, header.c_str());
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);

        // Set the request body
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());

        // Set the callback function to write the response data
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // Perform the request
        CURLcode res = curl_easy_perform(curl);

        // Clean up
        curl_slist_free_all(headerList);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            std::cerr << "Error: " << curl_easy_strerror(res) << std::endl;
        }
    }

    return response;
}

// Function to send a GET request with headers
std::string sendGetRequest(const std::string& url, const std::vector<std::string>& headers) {
    CURL* curl = curl_easy_init();
    std::string response;

    if (curl) {
        // Set the URL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // Set the headers
        curl_slist* headerList = NULL;
        for (const std::string& header : headers) {
            headerList = curl_slist_append(headerList, header.c_str());
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);

        // Set the callback function to write the response data
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // Perform the request
        CURLcode res = curl_easy_perform(curl);

        // Clean up
        curl_slist_free_all(headerList);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            std::cerr << "Error: " << curl_easy_strerror(res) << std::endl;
        }
    }

    return response;
}

// ------------------------ HELPER FUNCTIONS -----------------------------


// ------------------------ AUTH -----------------------------
std::string getAccessToken(){

    // Request URL
    std::string url = "https://app.metabypass.tech/CaptchaSolver/oauth/token";
    
    // Request headers
    std::vector<std::string> headers = {
        "Content-Type: application/json",
        "Accept: application/json"
    };

    // Request body
    std::string body = "{\"client_id\": \"" + clientId + "\","
                       "\"client_secret\": \"" + clientSecret + "\","
                       "\"grant_type\": \"password\","
                       "\"username\": \"" + email + "\","
                       "\"password\": \"" + password + "\"}";

    // Send the POST request
    std::string response = sendPostRequest(url, headers, body);

    // Check if the "access_token" field exists in the response body
    std::size_t found = response.find("access_token");
    
    // Process the response based on the presence of the "access_token" field
    if (found != std::string::npos) {
        std::size_t tokenStart = response.find(':', found) + 2;
        std::size_t tokenEnd = response.find('"', tokenStart);
        std::string accessToken = response.substr(tokenStart, tokenEnd - tokenStart);

        bool check= writeFile(tokenFilePath,accessToken);

        if(check==true){
            return accessToken;
        }else{
            std::cout << "metabypass token file could not create" << std::endl ;
            return "-1";
        }
        
        
    } else {
        std::cout << "unauth" << std::endl ;
        return "-1";
    }
}
// ------------------------ AUTH -----------------------------


// ------------------------ IMAGE CAPTCHA ---------------------
std::string imageCaptcha(const std::string base64_encoded,const int numeric=0,const int minLen=0 , const int maxLen=0){

    // Request URL
    std::string url = "https://app.metabypass.tech/CaptchaSolver/api/v1/services/captchaSolver";
    std::string access_token;
    std::string result;

    if(fileExists(tokenFilePath)){
        access_token=getFileContents(tokenFilePath);
    }else{
        access_token=getAccessToken();
    }

    if(access_token=="-1"){
        return "-1";
    }

    
    // // Request headers
    std::vector<std::string> headers = {
        "Content-Type: application/json",
        "Accept: application/json",
        "Authorization: Bearer "+access_token,
    };

    // // Request body
    std::string body = "{\"image\": \"" + base64_encoded + "\",\"numeric\": \"" + std::to_string(numeric) + "\",\"min_len\": \"" + std::to_string(minLen) + "\",\"max_len\": \"" + std::to_string(maxLen) + "\"}";
    
    // Send the POST request
    std::string response = sendPostRequest(url, headers, body);


    // Check if the "access_token" field exists in the response body
    std::size_t found = response.find("status_code");
    
    // Process the response based on the presence of the "status_code" field
    if (found != std::string::npos) {

        std::size_t statusCodeStart = response.find(':', found) + 1;
        std::size_t statusCodeEnd = response.find(',', found);
        std::string statusCodeStr = response.substr(statusCodeStart, statusCodeEnd - statusCodeStart);
        int statusCode = std::stoi(statusCodeStr);

        //Success Response
        if (statusCode == 200) {

            //Parese Response
            nlohmann::json jsonResponse = nlohmann::json::parse(response);

            try{
                // Extract the desired field from the JSON response
                return jsonResponse["data"]["result"];
            }catch (const std::exception& e){
                std::cout << e.what() << std::endl;
                return "-1";
            }

        } else {
            std::string check= getAccessToken();

            if(check=="-1"){
                return "-1";
            }

            return imageCaptcha(base64_encoded);
        }
        
    } else {
        std::cout << "bad response" << std::endl ;
        return "-1";
    }
    
}
// ------------------------ IMAGE CAPTCHA ---------------------


// ------------------------ reCAPTCHA v2 ---------------------
std::string recaptchaV2(const std::string site_url,const std::string site_key){

    // Request URL
    std::string url = "https://app.metabypass.tech/CaptchaSolver/api/v1/services/bypassReCaptcha";
    std::string access_token;

    if(fileExists(tokenFilePath)){
        access_token=getFileContents(tokenFilePath);
    }else{
        access_token=getAccessToken();
    }

    if(access_token=="-1"){
        return "-1";
    }

    
    // // Request headers
    std::vector<std::string> headers = {
        "Content-Type: application/json",
        "Accept: application/json",
        "Authorization: Bearer "+access_token,
    };

    // Request body
    std::string body = "{\"url\": \"" + site_url + "\","
                       "\"version\": \"2\","
                       "\"sitekey\": \"" + site_key + "\"}";

    // Send the POST request
    std::string response = sendPostRequest(url, headers, body);

    // Check if the "access_token" field exists in the response body
    std::size_t found = response.find("status_code");
    
    // Process the response based on the presence of the "status_code" field
    if (found != std::string::npos) {
        std::size_t statusCodeStart = response.find(':', found) + 1;
        std::size_t statusCodeEnd = response.find(',', found);
        std::string statusCodeStr = response.substr(statusCodeStart, statusCodeEnd - statusCodeStart);
        int statusCode = std::stoi(statusCodeStr);

        if (statusCode == 200) {
            return response;
        } else {
            std::string check= getAccessToken();
            if(check=="-1"){
                return "-1";
            }

            return recaptchaV2(site_url,site_key);
        }
        
    } else {
        std::cout << "bad response" << std::endl;
        return "-1";
    }
    
}



std::string getResult(const int recaptcha_id){


    std::string access_token;

    if(fileExists(tokenFilePath)){
        access_token=getFileContents(tokenFilePath);
    }else{
        access_token=getAccessToken();
    }

    if(access_token=="-1"){
        return "-1";
    }

    
    // // Request headers
    std::vector<std::string> headers = {
        "Content-Type: application/json",
        "Accept: application/json",
        "Authorization: Bearer "+access_token,
    };


    // Request URL
    std::string url = "https://app.metabypass.tech/CaptchaSolver/api/v1/services/getCaptchaResult?recaptcha_id="+ std::to_string(recaptcha_id);

    // Send the POST request
    std::string response = sendGetRequest(url, headers);

    // Check if the "access_token" field exists in the response body
    std::size_t found = response.find("status_code");
    
    // Process the response based on the presence of the "status_code" field
    if (found != std::string::npos) {
        std::size_t statusCodeStart = response.find(':', found) + 1;
        std::size_t statusCodeEnd = response.find(',', found);
        std::string statusCodeStr = response.substr(statusCodeStart, statusCodeEnd - statusCodeStart);
        int statusCode = std::stoi(statusCodeStr);

        if (statusCode == 200) {
            return response;
        }else if(statusCode == 201){
           
            return "1";
        
        } else {
            std::string check= getAccessToken();
            if(check=="-1"){
                return "-1";
            }

            return getResult(recaptcha_id);
        }
        
    } else {
        return "-1";
    }
    
}
// ------------------------ reCAPTCHA v2 ---------------------


// ------------------------ reCAPTCHA v2 Handler ---------------------
std::string recaptchaV2Handler(const std::string site_url , const std::string site_key) {

    int recaptcha_id;

    //Do Request to get recaptcha id
    std::string response=recaptchaV2(site_url,site_key);

    //Check Result
    if(response=="-1"){
        return "-1";
    }

    //Parse Response
    nlohmann::json jsonResponse = nlohmann::json::parse(response);

    try{
        // Extract the desired field from the JSON response
        recaptcha_id = jsonResponse["data"]["RecaptchaId"];
    }catch (const std::exception& e){
        std::cout << e.what() << std::endl;
        return "-1";
    }



    if (recaptcha_id) {

        //handle get result wait 20 - 60 seconds 
        for (int i = 0; i < 6; ++i) {

            // Sleep for 10 seconds
            std::this_thread::sleep_for(std::chrono::seconds(10));

            //request to get recaptcha result
            std::string result = getResult(recaptcha_id);

            //check result
            if (!result.empty()) {

                if(result=="1"){
                    continue;
                }else if(result=="-1"){
                    return "-1";
                }else {
                    
                    nlohmann::json jsonResponse = nlohmann::json::parse(result);

                    try{
                        // Extract the desired field from the JSON response
                        result = jsonResponse["data"]["RecaptchaResponse"];

                    }catch (const std::exception& e){
                        std::cout << e.what() << std::endl;
                        return "-1";
                    }
                    return result;
                }

            } else {
                std::cout << "Error" << std::endl;
                return "-1";
            }
        }
        
    } 

    std::cout << "bad response" << std::endl;
    return "-1";
    

}
// ------------------------ reCAPTCHA v2 Handler ---------------------


// ------------------------ reCAPTCHA v3 Handler ---------------------
std::string recaptchaV3(const std::string site_url,const std::string site_key){

    // Request URL
    std::string url = "https://app.metabypass.tech/CaptchaSolver/api/v1/services/bypassReCaptcha";
    std::string access_token;

    if(fileExists(tokenFilePath)){
        access_token=getFileContents(tokenFilePath);
    }else{
        access_token=getAccessToken();
    }

    if(access_token=="-1"){
        return "-1";
    }

    
    // // Request headers
    std::vector<std::string> headers = {
        "Content-Type: application/json",
        "Accept: application/json",
        "Authorization: Bearer "+access_token,
    };

    // Request body
    std::string body = "{\"url\": \"" + site_url + "\","
                       "\"version\": \"3\","
                       "\"sitekey\": \"" + site_key + "\"}";

    // Send the POST request
    std::string response = sendPostRequest(url, headers, body);

    // Check if the "access_token" field exists in the response body
    std::size_t found = response.find("status_code");
    
    // Process the response based on the presence of the "status_code" field
    if (found != std::string::npos) {
        std::size_t statusCodeStart = response.find(':', found) + 1;
        std::size_t statusCodeEnd = response.find(',', found);
        std::string statusCodeStr = response.substr(statusCodeStart, statusCodeEnd - statusCodeStart);
        int statusCode = std::stoi(statusCodeStr);

        if (statusCode == 200) {
            //Parese Response
            nlohmann::json jsonResponse = nlohmann::json::parse(response);

            try{
                // Extract the desired field from the JSON response
                return jsonResponse["data"]["RecaptchaResponse"];
            }catch (const std::exception& e){
                std::cout << e.what() << std::endl;
                return "-1";
            }
        } else {
            std::string check= getAccessToken();
            if(check=="-1"){
                return "-1";
            }

            return recaptchaV3(site_url,site_key);
        }
        
    } 

    std::cout << "bad response" << std::endl;
    return "-1";

    
}
// ------------------------ reCAPTCHA v3 Handler ---------------------


// ------------------------ reCAPTCHA v3 Handler ---------------------
std::string recaptchaInvisible(const std::string site_url,const std::string site_key){

    // Request URL
    std::string url = "https://app.metabypass.tech/CaptchaSolver/api/v1/services/bypassReCaptcha";
    std::string access_token;

    if(fileExists(tokenFilePath)){
        access_token=getFileContents(tokenFilePath);
    }else{
        access_token=getAccessToken();
    }

    if(access_token=="-1"){
        return "-1";
    }

    
    // // Request headers
    std::vector<std::string> headers = {
        "Content-Type: application/json",
        "Accept: application/json",
        "Authorization: Bearer "+access_token,
    };

    // Request body
    std::string body = "{\"url\": \"" + site_url + "\","
                       "\"version\": \"3\","
                       "\"sitekey\": \"" + site_key + "\"}";

    // Send the POST request
    std::string response = sendPostRequest(url, headers, body);

    // Check if the "access_token" field exists in the response body
    std::size_t found = response.find("status_code");
    
    // Process the response based on the presence of the "status_code" field
    if (found != std::string::npos) {
        std::size_t statusCodeStart = response.find(':', found) + 1;
        std::size_t statusCodeEnd = response.find(',', found);
        std::string statusCodeStr = response.substr(statusCodeStart, statusCodeEnd - statusCodeStart);
        int statusCode = std::stoi(statusCodeStr);

        if (statusCode == 200) {
            //Parese Response
            nlohmann::json jsonResponse = nlohmann::json::parse(response);

            try{
                // Extract the desired field from the JSON response
                return jsonResponse["data"]["RecaptchaResponse"];
            }catch (const std::exception& e){
                std::cout << e.what() << std::endl;
                return "-1";
            }
        } else {
            std::string check= getAccessToken();
            if(check=="-1"){
                return "-1";
            }

            return recaptchaInvisible(site_url,site_key);
        }
        
    } 

    std::cout << "bad response" << std::endl;
    return "-1";

    
}
// ------------------------ reCAPTCHA Invisible Handler ---------------------
