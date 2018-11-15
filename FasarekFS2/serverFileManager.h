
bool isServerDeleteable(String filename) {
  if (filename == "config.json"
    ||filename == "template.html"
    ||filename == "ux.html")
  {
    return false;
  } 
  return true;
}
  
bool isServerListable(char* filename) {
  int8_t len = strlen(filename);
  bool result;
  if (  strstr(strlwr(filename + (len - 4)), ".jpg")
     || strstr(strlwr(filename + (len - 5)), ".json")
    ) {
    result = true;
  } else {
    result = false;
  }
  return result;
}


void serverListFiles() {
  String fileName = "/template.html";
  webTemplate = "";
  
  if (SPIFFS.exists(fileName)) {
    File file = SPIFFS.open(fileName, "r");
    //server.streamFile(file, getContentType(fileName));
    
    while (file.available() != 0) {  
      webTemplate += file.readStringUntil('\n');  
    }
    file.close();
  } else {
    Serial.println("Could not read "+fileName+" from SPIFFS");
    server.send(200, "text/html", "Could not read "+fileName+" from SPIFFS");
    return;
  }
  
  String body = "<table class='table'>";
  body += "<tr><th>File</th><th>Size</th><th>Del</th></tr>";
  
  Dir dir = SPIFFS.openDir("/");
  String fileUnit;
  unsigned int fileSize;
  char fileChar[32];
  
  while (dir.next()) {
    String fileName = dir.fileName();
    fileName.toCharArray(fileChar, 32);
    if (!isServerListable(fileChar)) continue;
    
    if (dir.fileSize()<1024) {
        fileUnit = " bytes";
        fileSize = dir.fileSize();
        } else {
          fileUnit = " Kb";
          fileSize = dir.fileSize()/1024;
        }
    fileName.remove(0,1);
    body += "<tr><td><a href='/fs/download?f="+fileName+"'>";
    body += fileName+"</a></td>";
    body += "<td>"+ String(fileSize)+fileUnit +"</td>";
    body += "<td>";
    if (isServerDeleteable(fileName)) {
      body += "<a class='btn-sm btn-danger' href='/fs/delete?f="+fileName+"'>x</a>";
    }
    body += "</td>";
    body += "</tr>";
  }
    
  body += "</table>";

  FSInfo fs_info;
  SPIFFS.info(fs_info);
  body += "<br>Total KB: "+String(fs_info.totalBytes/1024)+" Kb";
  body += "<br>Used KB: "+String(fs_info.usedBytes/1024)+" Kb";
  body += "<br>Avail KB: <b>"+String((fs_info.totalBytes-fs_info.usedBytes)/1024)+" Kb</b><br>";

  webTemplate.replace("{{localDomain}}", localDomain);
  webTemplate.replace("{{home}}", "Camera UI");
  webTemplate.replace("{{body}}", body);
  
  server.send(200, "text/html", webTemplate);
}

void serverDownloadFile() {
  if (server.args() > 0 ) { 
    if (server.hasArg("f")) {
      String filename = server.arg(0);
      File download = SPIFFS.open("/"+filename, "r");
      if (download) {
        server.sendHeader("Content-Type", "text/text");
        server.sendHeader("Content-Disposition", "attachment; filename="+filename);
        server.sendHeader("Connection", "close");
        server.streamFile(download, "application/octet-stream");
        download.close();
      } else {
        server.send(404, "text/html", "file: "+ filename +" not found.");
      }
    } else {
      server.send(404, "text/html", "f parameter not received by GET.");
    }
  } else {
    server.send(404, "text/html", "No server parameters received.");
  }
}

void serverDeleteFile() {
  if (server.args() > 0 ) { 
    if (server.hasArg("f")) {
      String filename = server.arg(0);
      if(isServerDeleteable(filename)) {
         SPIFFS.remove("/"+filename);
      }
      server.sendHeader("Location", "/fs/list", true);
      server.send (302, "text/plain", "");
    } else {
      server.send(404, "text/html", "f parameter not received by GET.");
    }
  }
}

