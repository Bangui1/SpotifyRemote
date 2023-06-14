#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Base64.h>
#include <EEPROM.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include "settings.h"
#include "spotifyClient.h"




#define recieverPin 21




AsyncWebServer server(80);

SpotifyClient spotify = SpotifyClient(clientId, clientSecret, refreshToken);


//authenticate to spotify, logging in and allowing the app to access the data
bool authenticateSpotify(String code) {
  // Exchange authorization code for access token
  HTTPClient http;
  String payload = "grant_type=authorization_code&code=" + code + "&redirect_uri=" + redirectUri;
  String auth = clientId + ":" + clientSecret;
  String encodedAuth = base64::encode(auth);

  http.begin("https://accounts.spotify.com/api/token");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.addHeader("Authorization", "Basic " + encodedAuth);
  int httpCode = http.POST(payload);

  if (httpCode == 200) {
    DynamicJsonDocument jsonDoc(512);
    deserializeJson(jsonDoc, http.getString());

    const char* accessToken = jsonDoc["access_token"];
    const char* refreshToken1 = jsonDoc["refresh_token"];

    Serial.println("accesstoken: " + String(accessToken));
    Serial.println("refreshToken: " + String(refreshToken1));
    
    // Store access token and refresh token in non-volatile memory or EEPROM

    http.end();
    return true;
  } else {
    http.end();
    return false;
  }
}



IRrecv irReciever(recieverPin);
decode_results irResults;
String trackId;

void setup() {
  Serial.begin(115200);

  //setup receiver
  irReciever.enableIRIn();
  
  //connect to wifi
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  

//webpage button to login to spotify
//scopes used: user-read-private(access private playlists) user-read-email(required for it to work)
//user-modify-playback-state(access to the player) user-library-modify(for liking songs) user-read-playback-state(for reading if paused, shuffle, etc.)
/**
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<html><body>";
    html += "<a href=\"https://accounts.spotify.com/authorize?response_type=code&client_id=";
    html += clientId;
    html += "&redirect_uri=";
    html += redirectUri;
    html += "&scope=user-read-private%20user-read-email%20user-modify-playback-state%20user-library-modify%20user-read-playback-state\">Click here to login with Spotify</a>";
    html += "</body></html>";
    request->send(200, "text/html", html);
  });



  server.on("/auth/callback", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("code")) {
      String code = request->getParam("code")->value();
      Serial.println("code:" + code);
      bool auth = authenticateSpotify(code);
      if (auth) {
        request->send(200, "text/plain", "Authentication successful");
        Serial.println("successful");
      } else {
        request->send(500, "text/plain", "Authentication failed");
        Serial.print("auth failed");
      }
    } else {
      request->send(400, "text/plain", "Invalid request");
      Serial.println("invalid");
    }
  });
  **/

  //server.begin();


  spotify.FetchToken();
  
  spotify.isPlaying();
}




void togglePlayback(){
  if(spotify.isPlaying()){
    spotify.Pause();
  } else {
    spotify.Play();
  }
}


void loop() {
  
  if(irReciever.decode(&irResults)){
    Serial.println(irResults.value, HEX);

    switch (irResults.value){
      case 0xFF38C7: //ok
        togglePlayback();
        break;
      case 0xFF5AA5: //right arrow
        spotify.SkipNext();
        break;
      case 0xFF10EF: //left arrow
        spotify.SkipPrevious();
        break;
      case 0xFF6897: //*
        spotify.ToggleShuffle();
        break;
      case 0xFFB04F: //#
        spotify.ToggleRepeat();
        break;
      case 0xFFA25D: //1
        spotify.PlayContext("spotify:playlist:5zA2nl2MzTUDDzV0bQ4DVT");
        break;
      case 0xFF18E7: //up arrow
        spotify.VolumeUp();
        break;
      case 0xFF4AB5: //down arrow
        spotify.VolumeDown();
        break;
      case 0xFF9867: //0
        trackId = spotify.GetCurrentTrack();
        spotify.LikeCurrentSong(trackId);
        break;
      case 0xFF629D: //2
        spotify.PlayContext("spotify:album:5VIQ3VaAoRKOEpJ0fewdvo");
        break;
      case 0xFFE21D: //3
        spotify.PlayContext("spotify:album:3xB3SzIEkry77YmregfUHZ");
        break;
      case 0xFF22DD://4
        spotify.PlayContext("spotify:album:5wtE5aLX5r7jOosmPhJhhk");
        break;
      case 0xFF02FD://5
        spotify.PlayContext("spotify:playlist:7xRSfX4ablccdanZY70lZ4");
        break;
      case 0xFFC23D://6
        spotify.PlayContext("spotify:album:0JeyP8r2hBxYIoxXv11XiX");
        break;
      case 0xFFE01F://7
        spotify.PlayTrack("spotify:track:0n89qPsABbCIG3psemdvfT");
        break;
      case 0xFFA857://8
        spotify.PlayContext("spotify:album:1weenld61qoidwYuZ1GESA");
        break;
      case 0xFF906F://9
        spotify.PlayTrack("spotify:track:21qnJAMtzC6S5SESuqQLEK");
        break;
      default:
      Serial.println("Unknown code");
      break;
    }

    irReciever.resume();
  }
  
}

