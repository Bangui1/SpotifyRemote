#include "spotifyClient.h"
#include <base64.h>
#include <HTTPClient.h>

String playUrl = "https://api.spotify.com/v1/me/player/play";
String pauseUrl = "https://api.spotify.com/v1/me/player/pause";
String playerUrl = "https://api.spotify.com/v1/me/player"; // returns player info
String skipNextUrl = "https://api.spotify.com/v1/me/player/next";
String skipPreviousUrl = "https://api.spotify.com/v1/me/player/previous";
String setVolumeUrl = "https://api.spotify.com/v1/me/player/volume?volume_percent=";
String setShuffleUrl = "https://api.spotify.com/v1/me/player/shuffle?state=";
String setRepeatUrl = "https://api.spotify.com/v1/me/player/repeat?state=";
String currentPlayingUrl = "https://api.spotify.com/v1/me/player/currently-playing";
String LikeUrl = "https://api.spotify.com/v1/me/tracks?ids=";

SpotifyClient::SpotifyClient(String clientId, String clientSecret, String refreshToken){
    this->clientId = clientId;
    this->clientSecret = clientSecret;
    this->refreshToken = refreshToken;

    client.setCACert(digicert_root_ca);
}

void SpotifyClient::FetchToken(){
    HTTPClient http;

    String body = "grant_type=refresh_token&refresh_token=" + refreshToken;
    String authorizationRaw = clientId + ":" + clientSecret;
    String authorization = base64::encode(authorizationRaw);
    http.begin(client, "https://accounts.spotify.com/api/token");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.addHeader("Authorization", "Basic " + authorization);

    Serial.println("here");

    int httpCode = http.POST(body);
    Serial.println(httpCode);

    if(httpCode > 0){
        String returnedPayload = http.getString();
        Serial.println(returnedPayload);
        if(httpCode == 200){
            Serial.println("here");
            DynamicJsonDocument jsonDoc(1024);
            DeserializationError error = deserializeJson(jsonDoc, returnedPayload);
            if (error) {
                Serial.print("deserializeJson() failed: ");
                Serial.println(error.c_str());
            } else {
                accessToken = jsonDoc["access_token"].as<String>();
                Serial.println("and here");
                Serial.println("Got new Access Token");
            }


        } else{
            Serial.println("Failed to get new Access Token");
            Serial.println(httpCode);
            Serial.println(returnedPayload);
        }
    }
    else{
        Serial.println("failed to connet to spotify");
    }
    http.end();

}




int SpotifyClient::Pause(){
    HttpResult result;
    result.httpCode = 0;

    HTTPClient http;
    http.begin(client, pauseUrl);

    String authorization = "Bearer " + accessToken;
    http.addHeader(F("Content-Type"), "application/json");
    http.addHeader(F("Authorization"), authorization);
    http.addHeader(F("Content-Length"), String(0));
    result.httpCode = http.PUT("");

    if(result.httpCode > 0){
        Serial.println(result.httpCode);
        if(result.httpCode == 401){
            FetchToken();
            return Pause();
        }
        if(http.getSize() > 0){
            result.payload = http.getString();
        }
    } else {
        Serial.print("Failed to connect to " + pauseUrl);
    }

    http.end();
    return result.httpCode;
}

int SpotifyClient::Play(){
    HttpResult result;
    result.httpCode = 0;

    HTTPClient http;
    http.begin(client, playUrl);

    String authorization = "Bearer " + accessToken;
    http.addHeader(F("Content-Type"), "application/json");
    http.addHeader(F("Authorization"), authorization);
    http.addHeader(F("Content-Length"), String(0));
    result.httpCode = http.PUT("");

    if(result.httpCode > 0){
        Serial.println(result.httpCode);
        if(result.httpCode == 401){
            FetchToken();
            return Play();
        }
        if(http.getSize() > 0){
            result.payload = http.getString();
        }
    } else {
        Serial.print("Failed to connect to " + playUrl);
    }

    http.end();
    return result.httpCode;
}

bool SpotifyClient::isPlaying(){
    HttpResult result;
    result.httpCode = 0;
    bool playing = false;

    HTTPClient http;
    http.begin(client, playerUrl);

    String authorization = "Bearer " + accessToken;
    http.addHeader(F("Content-Type"), "application/json");
    http.addHeader(F("Authorization"), authorization);
    http.addHeader(F("Content-Length"), String(0));
    result.httpCode = http.GET();

    if(result.httpCode > 0){
        Serial.println(result.httpCode);
        String returnedPayload = http.getString();
        if(result.httpCode == 401){
            FetchToken();
            return isPlaying();
        }
        if(result.httpCode == 200){
            DynamicJsonDocument jsonDoc(10240);
            DeserializationError error = deserializeJson(jsonDoc, returnedPayload);
            if (error) {
                Serial.print("deserializeJson() failed: ");
                Serial.println(error.c_str());
            } else {
                volume = jsonDoc["device"]["volume_percent"];
                shuffle = jsonDoc["shuffle_state"];
                repeat = jsonDoc["repeat_state"].as<String>();
                return jsonDoc["is_playing"];

            }
        }
        if(http.getSize() > 0){
            result.payload = http.getString();
        }
    } else {
        Serial.print("Failed to connect to " + playerUrl);
    }

    http.end();
    return playing;
}

int SpotifyClient::SkipNext(){
    HttpResult result;
    result.httpCode = 0;

    HTTPClient http;
    http.begin(client, skipNextUrl);

    String authorization = "Bearer " + accessToken;
    http.addHeader(F("Content-Type"), "application/json");
    http.addHeader(F("Authorization"), authorization);
    http.addHeader(F("Content-Length"), String(0));
    result.httpCode = http.POST("");

    if(result.httpCode > 0){
        Serial.println(result.httpCode);
    }
    http.end();
    return result.httpCode;
}

int SpotifyClient::SkipPrevious(){
    HttpResult result;
    result.httpCode = 0;

    HTTPClient http;
    http.begin(client, skipPreviousUrl);

    String authorization = "Bearer " + accessToken;
    http.addHeader(F("Content-Type"), "application/json");
    http.addHeader(F("Authorization"), authorization);
    http.addHeader(F("Content-Length"), String(0));
    result.httpCode = http.POST("");

    if(result.httpCode > 0){
        Serial.println(result.httpCode);
    }
    http.end();
    return result.httpCode;
}


int SpotifyClient::VolumeUp(){
    volume += 10;
    if (volume > 100){
        volume = 100;
    }
    
    HttpResult result;
    result.httpCode = 0;

    HTTPClient http;
    http.begin(client, setVolumeUrl+volume);

    String authorization = "Bearer " + accessToken;
    http.addHeader(F("Content-Type"), "application/json");
    http.addHeader(F("Authorization"), authorization);
    http.addHeader(F("Content-Length"), String(0));
    result.httpCode = http.PUT("");

    if(result.httpCode > 0){
        Serial.println(result.httpCode);
    }
    http.end();
    return result.httpCode;

}

int SpotifyClient::VolumeDown(){
    volume -= 10;
    if (volume < 0){
        volume = 0;
    }
    
    HttpResult result;
    result.httpCode = 0;

    HTTPClient http;
    http.begin(client, setVolumeUrl+volume);

    String authorization = "Bearer " + accessToken;
    http.addHeader(F("Content-Type"), "application/json");
    http.addHeader(F("Authorization"), authorization);
    http.addHeader(F("Content-Length"), String(0));
    result.httpCode = http.PUT("");

    if(result.httpCode > 0){
        Serial.println(result.httpCode);
    }
    http.end();
    return result.httpCode;
}

int SpotifyClient::ToggleShuffle(){

    shuffle = !shuffle;

    String isShuffle;
    if(shuffle){
        isShuffle = "true";
    } else{
        isShuffle = "false";
    }

    HttpResult result;
    result.httpCode = 0;

    HTTPClient http;
    http.begin(client, setShuffleUrl+isShuffle);

    String authroization = "Bearer " + accessToken;
    http.addHeader(F("Content-Type"), "application/json");
    http.addHeader(F("Authorization"), authroization);
    http.addHeader(F("Content-Length"), String(0));
    result.httpCode = http.PUT("");

    if(result.httpCode > 0){
        Serial.println(result.httpCode);
        if(result.httpCode != 204){
            shuffle = !shuffle;
        }
    }
    http.end();
    return result.httpCode;
}

int SpotifyClient::ToggleRepeat(){
    if (repeat == "track") {
        repeat = "context";
    } else if (repeat == "context") {
        repeat = "off";
    } else if (repeat == "off") {
        repeat = "track";
    } else {
        repeat = "off";
    }

    HttpResult result;
    result.httpCode = 0;

    HTTPClient http;
    http.begin(client, setRepeatUrl+repeat);

    String authroization = "Bearer " + accessToken;
    http.addHeader(F("Content-Type"), "application/json");
    http.addHeader(F("Authorization"), authroization);
    http.addHeader(F("Content-Length"), String(0));
    result.httpCode = http.PUT("");

    if(result.httpCode > 0){
        Serial.println(result.httpCode);
    }
    http.end();
    return result.httpCode;
}

int SpotifyClient::PlayContext(String contextUri){
    HttpResult result;
    result.httpCode = 0;

    HTTPClient http;
    http.begin(client, playUrl);


    String body = "{\"context_uri\":\"" + contextUri + "\",\"offset\":{\"position\":0,\"position_ms\":0}}";
    String authorization = "Bearer " + accessToken;
    http.addHeader(F("Content-Type"), "application/json");
    http.addHeader(F("Authorization"), authorization);

    result.httpCode = http.PUT(body);

    if(result.httpCode > 0){
        Serial.println(result.httpCode);
    }
    http.end();
    return result.httpCode;
}

int SpotifyClient::PlayTrack(String trackUri){
    HttpResult result;
    result.httpCode = 0;

    HTTPClient http;
    http.begin(client, playUrl);

    String body = "{\"uris\":[\"" + trackUri + "\"],\"offset\":{\"position\":0,\"position_ms\":0}}";
    String authorization = "Bearer " + accessToken;
    http.addHeader(F("Content-Type"), "application/json");
    http.addHeader(F("Authorization"), authorization);

    result.httpCode = http.PUT(body);

    if(result.httpCode > 0){
        Serial.println(result.httpCode);
    }
    http.end();
    return result.httpCode;
}



void SpotifyClient::LikeCurrentSong(String trackId){
    HttpResult result;
    result.httpCode = 0;

    HTTPClient http;
    FetchToken();
    http.begin(client, LikeUrl + trackId);

    String authorization = "Bearer " + accessToken;
    http.addHeader(F("Content-Type"), "application/json");
    http.addHeader(F("Authorization"), authorization);

    StaticJsonDocument<128> requestBody;
    requestBody["ids"].add("string");

    String requestBodyString;
    serializeJson(requestBody, requestBodyString);

    result.httpCode = http.PUT(requestBodyString);

    if (result.httpCode > 0) {
        Serial.println(result.httpCode);
        if (result.httpCode != 204) {
            String errorPayload = http.getString();
            Serial.println("Error payload: " + errorPayload);
        }
    } else {
        Serial.println("Failed to connect to " + LikeUrl);
    }

    http.end();
}

String SpotifyClient::GetCurrentTrack(){
    HttpResult result;
    result.httpCode = 0;

    String trackId = "";
    HTTPClient http;
    http.begin(client, currentPlayingUrl);
    String authorization = "Bearer " + accessToken;
    http.addHeader(F("Content-Type"), "application/json");
    http.addHeader(F("Authorization"), authorization);

    result.httpCode = http.GET();

    if(result.httpCode > 0){
        Serial.println(result.httpCode);
        String returnedPayload = http.getString();
        if(result.httpCode == 401){
            FetchToken();
            return GetCurrentTrack();
        }
        if(result.httpCode == 200){
            DynamicJsonDocument jsonDoc(10240);
            DeserializationError error = deserializeJson(jsonDoc, returnedPayload);
            if (error) {
                Serial.print("deserializeJson() failed: ");
                Serial.println(error.c_str());
            } else {
                trackId = jsonDoc["item"]["id"].as<String>();
                Serial.println(trackId);
                return trackId;
            }
        }
        if(http.getSize() > 0){
            result.payload = http.getString();
        }
    } else {
        Serial.print("Failed to connect to " + currentPlayingUrl);
    }

    http.end();
    Serial.println(trackId);
    return trackId;
}

