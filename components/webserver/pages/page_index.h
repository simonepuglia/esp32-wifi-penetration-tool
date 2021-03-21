#ifndef PAGE_INDEX_H
#define PAGE_INDEX_H

const char page_index[] =
"<!DOCTYPE html>\n"
"<html>\n"
"<head>\n"
    "<title>ESP32 Management AP</title>\n"
    "<style>\n"
        "table, tr, th {\n"
            "border: 1px solid;\n"
            "border-collapse: collapse;\n"
            "text-align: center;\n"
        "}\n"
        "th, td {\n"
            "padding:10px;\n"
        "}\n"
        "tr:hover, tr.selected {\n"
            "background-color: lightblue;\n"
            "cursor:pointer;\n"
        "}\n"
    "</style>\n"
"</head>\n"
"<body onLoad=\"getStatus()\">\n"
    "<h1>ESP32 Wi-Fi Penetration Tool</h1>\n"
    "<div id=\"errors\"></div>\n"
    "<div id=\"loading\">Loading... Please wait</div>\n"
    "<div id=\"ready\" style=\"display: none;\">\n"
        "<h2>Attack configuration</h2>\n"
        "<h3>Available APs:</h3>\n"
        "<table id=\"ap-list\"></table>\n"
        "<button type=\"button\" onClick=\"refreshAps()\">Refresh</button>\n"
        "<br>\n"
        "Attack type:\n"
        "<select id=\"attack_type\">\n"
            "<option value=\"0\">ATTACK_TYPE_PASSIVE</option>\n"
            "<option value=\"1\">ATTACK_TYPE_HANDSHAKE</option>\n"
            "<option value=\"2\" selected>ATTACK_TYPE_PMKID</option>\n"
        "</select>\n"
        "<br>\n"
        "Attack timeout (seconds):\n"
        "<input type=\"text\" id=\"attack_timeout\" value=\"5\"/>\n"
        "<br>\n"
        "<button type=\"button\" onClick=\"runAttack()\">Attack</button>\n"
    "</div>\n"
    "<div id=\"running\" style=\"display: none;\">\n"
        "Attack in progress.. <br>\n"
        "It may take few moments based on attack timeout.. <br>\n"
        "You may be kicked from management AP in the meantime.. <br>\n"
        "Please reconnect to get result. <br>\n"
        "Progress: <div id=\"running-progress\"></div>\n"
    "</div>\n"
    "<div id=\"result\" style=\"display: none;\">\n"
        "<div id=\"result-meta\">Loading result.. Please wait</div>\n"
        "<div id=\"result-content\"></div>\n"
        "<button type=\"button\" onClick=\"resetAttack()\">New attack</button>\n"
    "</div>\n"
    "<script>\n"
    "var selectedApElement = -1;\n"
    "var poll;\n"
    "var poll_interval = 1000;\n"
    "var defaultResultContent = document.getElementById(\"result\").innerHTML;\n"
    "function getStatus() {\n"
        "var oReq = new XMLHttpRequest();\n"
        "oReq.onload = function() {\n"
            "var arrayBuffer = oReq.response;\n"
            "if(arrayBuffer) {\n"
                "var byteArray = new Uint8Array(arrayBuffer);\n"
                "var status = \"ERROR: Cannot parse attack state.\";\n"
                "document.getElementById(\"loading\").style.display = \"none\";\n"
                "switch(byteArray[0]) {\n"
                    "case 0:\n"
                        "showAttackConfig();\n"
                        "break;\n"
                    "case 1:\n"
                        "document.getElementById(\"running\").style.display = \"block\";\n"
                        "document.getElementById(\"running-progress\").innerHTML += \"*\";\n"
                        "console.log(\"Poll\");\n"
                        "setTimeout(getStatus, poll_interval);\n"
                        "break;\n"
                    "case 2:\n"
                        "showResult(\"FINISHED\", byteArray);\n"
                        "break;\n"
                    "case 3:\n"
                        "showResult(\"TIMEOUT\", byteArray);\n"
                        "break;\n"
                    "default:\n"
                        "document.getElementById(\"errors\").innerHTML = \"Error loading attack status! Unknown state.\";\n"
                "}\n"
                "return;\n"
                "\n"
            "}\n"
        "};\n"
        "oReq.onerror = function() {\n"
            "console.log(\"Request error\");\n"
            "document.getElementById(\"errors\").innerHTML = \"Cannot reach ESP32. Check that you are connected to management AP. You might get disconnected during attack.\";\n"
            "getStatus();\n"
        "};\n"
        "oReq.ontimeout = function() {\n"
            "console.log(\"Request timeout\");\n"
            "getStatus();  \n"
        "};\n"
        "oReq.open(\"GET\", \"status\", true);\n"
        "oReq.responseType = \"arraybuffer\";\n"
        "oReq.send();\n"
    "}\n"
    "function showAttackConfig(){\n"
        "document.getElementById(\"ready\").style.display = \"block\";\n"
        "refreshAps();\n"
    "}\n"
    "function showResult(status, byteArray){\n"
        "clearInterval(poll);\n"
        "document.getElementById(\"result\").innerHTML = defaultResultContent;\n"
        "document.getElementById(\"running\").style.display = \"none\";\n"
        "document.getElementById(\"result\").style.display = \"block\";\n"
        "document.getElementById(\"result-meta\").innerHTML = status + \"<br>\";\n"
        "type = \"ERROR: Cannot parse attack type.\";\n"
        "switch(byteArray[1]) {\n"
            "case 0:\n"
                "type = \"ATTACK_TYPE_PASSIVE\";\n"
                "break;\n"
            "case 1:\n"
                "type = \"ATTACK_TYPE_HANDSHAKE\";\n"
                "break;\n"
            "case 2:\n"
                "type = \"ATTACK_TYPE_PMKID\";\n"
                "let index = 3;\n"
                "var mac_ap = \"\";\n"
                "var mac_sta = \"\";\n"
                "var ssid = \"\";\n"
                "var pmkid = \"\";\n"
                "for(let i = 0; i < 6; i = i + 1) {\n"
                    "mac_ap += byteArray[index + i].toString(16);\n"
                "}\n"
                "\n"
                "index = index + 6;\n"
                "for(let i = 0; i < 6; i = i + 1) {\n"
                    "mac_sta += byteArray[index + i].toString(16);\n"
                "}\n"
                "index = index + 6;\n"
                "for(let i = 0; i < byteArray[index]; i = i + 1) {\n"
                    "ssid += byteArray[index + 1 + i].toString(16);\n"
                "}\n"
                "index = index + byteArray[index] + 1;\n"
                "var pmkid_cnt = 0;\n"
                "for(let i = 0; i < byteArray[2] - index + 3; i = i + 1) {\n"
                    "if((i % 16) == 0){\n"
                        "pmkid += \"<br>\";\n"
                        "pmkid += \"</code>PMKID #\" + pmkid_cnt + \": <code>\";\n"
                        "pmkid_cnt += 1;\n"
                    "}\n"
                    "pmkid += byteArray[index + i].toString(16);\n"
                "}\n"
                "document.getElementById(\"result-content\").innerHTML = \"\";\n"
                "document.getElementById(\"result-content\").innerHTML += \"MAC AP: <code>\" + mac_ap + \"</code><br>\";\n"
                "document.getElementById(\"result-content\").innerHTML += \"MAC STA: <code>\" + mac_sta + \"</code><br>\";\n"
                "document.getElementById(\"result-content\").innerHTML += \"(E)SSID: <code>\" + ssid + \"</code>\";\n"
                "document.getElementById(\"result-content\").innerHTML += \"<code>\" + pmkid + \"</code><br>\";\n"
                "document.getElementById(\"result-content\").innerHTML += \"<br>Hashcat ready format:\"\n"
                "document.getElementById(\"result-content\").innerHTML += \"<code>\" + pmkid + \"*\" + mac_ap + \"*\" + mac_sta  + \"*\" + ssid  + \"</code><br>\";\n"
                "break;\n"
            "default:\n"
                "type = \"UNKNOWN\";\n"
        "}\n"
        "document.getElementById(\"result-meta\").innerHTML += type + \"<br>\";\n"
    "}\n"
    "function refreshAps() {\n"
        "document.getElementById(\"ap-list\").innerHTML = \"Loading (this may take a while)...\";\n"
        "var oReq = new XMLHttpRequest();\n"
        "oReq.onload = function() {\n"
            "document.getElementById(\"ap-list\").innerHTML = \"<th>SSID</th><th>BSSID</th><th>RSSI</th>\";\n"
            "var arrayBuffer = oReq.response;\n"
            "if(arrayBuffer) {\n"
                "var byteArray = new Uint8Array(arrayBuffer);\n"
                "for  (let i = 0; i < byteArray.byteLength; i = i + 40) {\n"
                    "var tr = document.createElement('tr');\n"
                    "tr.setAttribute(\"id\", i / 40);\n"
                    "tr.setAttribute(\"onClick\", \"selectAp(this)\");\n"
                    "var td_ssid = document.createElement('td');\n"
                    "var td_rssi = document.createElement('td');\n"
                    "var td_bssid = document.createElement('td');\n"
                    "td_ssid.innerHTML = new TextDecoder(\"utf-8\").decode(byteArray.subarray(i + 0, i + 32));\n"
                    "tr.appendChild(td_ssid);\n"
                    "for(let j = 0; j < 6; j++){\n"
                        "td_bssid.innerHTML += byteArray[i + 33 + j].toString(16) + \":\";\n"
                    "}\n"
                    "tr.appendChild(td_bssid);\n"
                    "td_rssi.innerHTML = byteArray[i + 39] - 255;\n"
                    "tr.appendChild(td_rssi);\n"
                    "document.getElementById(\"ap-list\").appendChild(tr);\n"
                "}\n"
            "}\n"
        "};\n"
        "oReq.onerror = function() {\n"
            "document.getElementById(\"ap-list\").innerHTML = \"ERROR\";\n"
        "};\n"
        "oReq.open(\"GET\", \"ap-list\", true);\n"
        "oReq.responseType = \"arraybuffer\";\n"
        "oReq.send();\n"
    "}\n"
    "function selectAp(el) {\n"
        "console.log(el.id);\n"
        "if(selectedApElement != -1){\n"
            "selectedApElement.classList.remove(\"selected\")\n"
        "}\n"
        "selectedApElement=el;\n"
        "el.classList.add(\"selected\");\n"
    "}\n"
    "function runAttack() {\n"
        "if(selectedApElement == -1){\n"
            "console.log(\"No AP selected. Attack not started.\");\n"
            "document.getElementById(\"errors\").innerHTML = \"No AP selected. Attack not started.\";\n"
            "return;\n"
        "}\n"
        "var arrayBuffer = new ArrayBuffer(3);\n"
        "var uint8Array = new Uint8Array(arrayBuffer);\n"
        "uint8Array[0] = parseInt(selectedApElement.id);\n"
        "uint8Array[1] = parseInt(document.getElementById(\"attack_type\").value);\n"
        "uint8Array[2] = parseInt(document.getElementById(\"attack_timeout\").value);\n"
        "var oReq = new XMLHttpRequest();\n"
        "oReq.open(\"POST\", \"run-attack\", true);\n"
        "oReq.send(arrayBuffer);\n"
        "document.getElementById(\"ready\").style.display = \"none\";\n"
        "document.getElementById(\"running\").style.display = \"block\";\n"
        "getStatus();\n"
    "}\n"
    "function resetAttack(){\n"
        "var oReq = new XMLHttpRequest();\n"
        "oReq.open(\"HEAD\", \"reset\", true);\n"
        "oReq.send();\n"
        "document.getElementById(\"result\").style.display = \"none\";\n"
        "showAttackConfig();\n"
    "}\n"
    "</script>\n"
"</body>\n"
"</html>\n"
;
#endif

