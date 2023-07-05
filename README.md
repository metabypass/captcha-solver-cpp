# MetaBypass ( AI Captcha Solver )
## C++ wrapper to work with [MetaBypass](https://metabypass.tech) services

Free demo (no credit card required) -> https://app.metabypass.tech/application

<br/>

### Features

Solve image captcha , reCaptcha v2 & v3 , invisible reCaptcha <br/>
Auto handler for reCaptcha v2 <br/>
The compiled version to use easily with the shell everywhere. [compied version repo](https://metabypass.tech) <br/>

<br/>
<br/>

## Notice
to request we used the curl library in codes. so make sure that the curl library has been installed on your machine. <br/>
when you want to complile your c++ code use -lcurl in your command. example :  g++ my_code.cpp -lcurl  -o compiled_code

<br/>
<br/>


## Usage

<br/>

at the begining of the metabypass.cpp file (inc/metabypass.cpp) , exists "Credentials" section <br>
fill these variables with your credentials <br>
to get credentials go to [Application](https://app.metabypass.tech/application) page on [MetaBypass](https://app.metabypass.tech) <br><br>

**Setup** <br />
 ```c++
//Credentials
std::string clientId = "YOUR_CLIENT_ID"; //change here
std::string clientSecret = "YOUR_CLIENT_SECRET"; //change here
std::string email = "YOUR_EMAIL"; //change here
std::string password = "YOUR_PASSWORD"; //change here
 ```
then you can include metabypass.cpp file to your code and use metabypass wrapper easily. see examples <br><br>

```c++
//main file
#include "inc/metabypass.cpp"

  // your codes

```
<br><br>

## Examples

<br>

**imageCaptcha** <br/>


```c++
//main file
#include "inc/metabypass.cpp"

int main() {

    //-------------- Image Captcah---------------

    int numeric=0; // optional parameter . check document : https://app.metabypass.tech/docs.html?#api_3
    int minLen=0; // optional parameter
    int maxLen=0; // optional parameter

    // pass your image captcha file path 
    std::string base64_encoded=convert_image_to_base64("samples/icaptcha1.jpg");

    //call function
    std::string result=imageCaptcha(base64_encoded,numeric,minLen,maxLen);

    //result
    std::cout << result << std::endl;

    //-------------- Image Captcah---------------

    return 0;
}

```
<br>

**reCAPTCHA v2** <br/>


```c++
//main file
#include "inc/metabypass.cpp"

int main() {

    //-------------- reCAPTCHA V2---------------

    //pass your sitekey & site url here
    std::string site_url="SITE_URL";
    std::string site_key="SITE_KEY";

    //call function
    std::string result=recaptchaV2Handler(site_url,site_key);

    //result
    std::cout << result << std::endl;

    //-------------- reCAPTCHA V2---------------

    return 0;
}

```

<br>

**reCAPTCHA v3** <br/>


```c++
//main file
#include "inc/metabypass.cpp"

int main() {

    //-------------- reCAPTCHA V3---------------

    //pass your sitekey & site url here
    std::string site_url="SITE_URL";
    std::string site_key="SITE_KEY";

    //call function
    std::string result=recaptchaV3(site_url,site_key);

    //result
    std::cout << result << std::endl;
    
    //-------------- reCAPTCHA V3---------------

    return 0;
}

```

<br>

**reCAPTCHA invisible** <br/>


```c++
//main file
#include "inc/metabypass.cpp"

int main() {

    //-------------- reCAPTCHA invisible---------------

    //pass your sitekey & site url here
    std::string site_url="SITE_URL";
    std::string site_key="SITE_KEY";

    //call function
    std::string result=recaptchaInvisible(site_url,site_key);

    //result
    std::cout << result << std::endl;
    
    //-------------- reCAPTCHA invisible---------------

    return 0;
}

```