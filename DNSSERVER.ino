#include <WiFi.h>
#include <esp_task_wdt.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <HTTPClient.h>
#include <EEPROM.h>
//#include <Preferences.h>

const byte DNS_PORT = 53;
WebServer server(80);
DNSServer dnsServer;
//ค่าเริมต้น
String ssid = "SENSER-AP";
String apPassword = "12345678";
const String defaultssid = "SENSER-AP";
const String defaultpass = "12345678";
char currentLang = 'E', savedLang;
//
String deviceId, lang1, lang0;
//
// pinและตัวแปรในส่วน hardware
const int pirsenser_pin = 21;
unsigned long lastEmailTime = 0;
const unsigned long emailCooldown = 10000;  //หน่วงเวลา 10วิ
//
#define EEPROM_SIZE 512
#define SSID_ADDR 0
#define PASS_ADDR 40
#define LANG_ADDR 80

#define WiFi_SSID_ADDR 120
#define WIFI_PASS_ADDR 160
#define EMAIL1_ADDR 200
#define EMAIL2_ADDR 240
#define EMAIL3_ADDR 280
#define EMAIL4_ADDR 320


String pageHeader = R"HTML(
<!DOCTYPE html>
<html>
<head>
<!-- <meta name= "viewport" content="width=device-width,initial-scale=1.0"> -->
<meta charset="utf-8">
<title>ESP32 Portal</title>
<style>
    body {
        margin: 0;
        font-family: Arial;
        text-align: center;
        background: white;            
        color: #333;
    }
    .container { padding-top: 30px; }
    .card {
        background: #ffffff;
        padding: 20px;
        margin: 20px auto;
        width: 90%;
        max-width: 420px;
        border-radius: 18px;
    }
    
    input {
        width: 100%;
        height: 50px;
        box-sizing: border-box;
        padding-left: 10px;
        padding-right: 10px;
        padding: 8px;
        border-radius: 8px;
        border: 2px solid #000;   /*สีกรอบเป็นสีดำ*/
        margin-bottom: 10px;
        font-size: 40px;

        
    }

   

    .btn-primary{
      background:#4e73df;
      color:white;
      border: none;
      border-radius: 8px;
      text-decoration:none;
      display: inline-block;
      font-weight: bold;
      cursor: pointer;
    }

    .btn-large{
      padding: 40px 70px;
      font-size: 30px;
      margin-bottom: 10px;
      width: 100%;
      box-sizing: border-box;
      
    }
    .btn-small{
      font-size: 18px;
      width: 100px;   /* กว้างคงที่ */
      height: 70px;   /* สูงคงที่ */      
      box-sizing: border-box;
      line-height:70px;
      padding: 0;
      text-align: center;
    }

    .lang{
      position:absolute;
      top: 10px;
      right: 10px;
    }

    .form-box{
      margin-top: 150px;
    }
    .form-box-welcome{
      margin-top:50px;
    }

    .modal {
      display: none;
      position: fixed;
      z-index: 999;
      left: 0; top: 0;
      width: 100%; height: 100%;
      background: rgba(0,0,0,0.5);
    }
    .modal-content{
      background:#fff;
      margin: 50% auto;
      padding: 20px;
      width: 90%;
      max-width: 400px;
      border-radius: 12px;
    }
    .modal-content input{
      font-size : 20px;
    }
    .modal-content button{
      margin: 35px;
    }


</style>

</head>
<body>
<div class='container'>
<div class='card'>

)HTML";





String pageFooter = R"HTML(
<br>


<br><br>



<div class="lang">

<a id="langT"class="btn-primary btn-small" href="/lang/th"></a>
<a id="langE"class="btn-primary btn-small" href="/lang/en"></a>

</div>
</body></html>
)HTML";




void handleLogin() {


  String html = pageHeader;
  html += "<script>var LANG = '" + String(currentLang) + "';</script>";
  html += R"HTML(
    <script>
     var T = {
    welcome: "ยินดีต้อนรับ",
    setting: "การตั้งค่า",
    
    connectInternet: "เชื่อมต่ออินเตอร์เน็ต",
    addEmail: "เพิ่ม Email",
    renameDevice: "เปลี่ยนชื่ออุปกรณ์",
    repassword: "เปลี่ยนรหัสผ่าน",
    resetDevice: "รีเซ็ตอุปกรณ์",
    
    statusinterneton:  "🟢เชื่อมต่ออินเทอร์เน็ต",
    statusinternetoff: "🔴ไม่เชื่อมต่ออินเทอร์เน็ต",
    
    langthai:"ไทย",
    langeng:"อังกฤษ",
   
    enterssid:"กรอก SSID",
    enterpassword:"กรอก Password",
    cancel:"ยกเลิก",
    connect:"เชื่อมต่อ",
    enterSSID: "กรุณาใส่ SSID",
    passShort: "รหัสผ่านต้องมีอย่างน้อย 8 ตัว",
 
    enteremail1:"อีเมล 1",
    enteremail2:"อีเมล 2",
    enteremail3:"อีเมล 3",
    enteremail4:"อีเมล 4",
    okaddemail:"ตกลง",
    cancelAddemail:"ยกเลิก",
    invalidemail:"❌ อีเมลไม่ถูกต้อง",
    pleaseenteremail:"กรุณากรอกอีเมล",
    saveemaildone:"✅ บันทึกอีเมลแล้ว",
   
    enterName: "กรอกชื่อ",
    okrename: "ตกลง",
    cancelrename:"ยกเลิก",
    pleaseentername:"กรุณากรอกชื่อ",
    savenamedone:"บันทึกชื่อสำเร็จ!",
    rebooting: "กำลังรีบูท...",
        
    enterPass: "กรอกรหัสผ่าน",
    okrepass:"ตกลง",
    cancelrepass:"ยกเลิก",
    savepassdone: "บันทึกรหัสผ่านสำเร็จ!",
    pleaseenterpass:"กรุณากรอกรหัสผ่าน",

    okreset: "ตกลง",
    cancelreset: "ยกเลิก",
    confirmReset: "คุณแน่ใจหรือไม่ว่าต้องการรีเซ็ต?",
    resetDone: "รีเซ็ตสำเร็จ!",
  };
  var E = {
    
    welcome: "Welcome",
    setting: "Setting",
    
    connectInternet: "Connect Internet",
    addEmail: "Add Email",
    renameDevice: "ReName Device",
    repassword: "RePassword Device",
    resetDevice: "Reset Device",
    
    statusinterneton:  "🟢Connect internet",
    statusinternetoff: "🔴Not connect internet",
    
    langthai:"THAI",
    langeng:"ENG",

    enterssid:"Enter SSID",
    enterpassword:"Enter Password",
    cancel:"Cancel",
    connect:"Connect",
    enterSSID: "Please enter SSID",
    passShort: "Password must be at least 8 characters",
       
    enteremail1:"Email 1",
    enteremail2:"Email 2",
    enteremail3:"Email 3",
    enteremail4:"Email 4",
    okaddemail:"OK",
    cancelAddemail:"Cancel",
    invalidemail:"❌ Email is incorrect",
    pleaseenteremail:"Please enter Email",
    saveemaildone:"✅ Email saved",

    enterName: "Enter Name",
    okrename: "OK",
    cancelrename:"Cancel",
    pleaseentername:"Please enter Name",
    savenamedone:"Save Name Complete!",
    rebooting: "Rebooting...",

    enterPass: "Enter Password",
    okrepass:"OK",
    cancelrepass:"Cancel",
    pleaseenterpass:"Please enter Password",
    savepassdone: "Save Password Complete!",

    okreset: "OK",
    cancelreset: "Cancel",
    confirmReset: "Are you sure you want to reset?",
    resetDone: "Reset Complete!"
  };
  //สร้างตัวแปรเปลี่ยนภาษา
  var L = (LANG === 'T')? T:E;

  window.onload = function(){  
    //btn
    document.getElementById("txtWelcome").innerText = L.welcome;
    document.getElementById("txtSetting").innerText = L.setting;
    document.getElementById("btnconnectWifi").innerText = L.connectInternet;
    document.getElementById("btnAddemail").innerText = L.addEmail;
    document.getElementById("btnRename").innerText = L.renameDevice;
    document.getElementById("btnRepass").innerText = L.repassword;
    document.getElementById("btnReset").innerText = L.resetDevice;
    //lang
    document.getElementById("langT").innerText = L.langthai;
    document.getElementById("langE").innerText = L.langeng;
    //overlayของ connectinternet
    document.getElementById("netStatus").innerText = L.statusinternetoff;
    document.getElementById("txtconnectWifi").innerText = L.connectInternet;
    document.getElementById("wifi_ssid").placeholder = L.enterssid;
    document.getElementById("wifi_pass").placeholder = L.enterpassword;
    document.getElementById("btncancel").innerText = L.cancel;
    document.getElementById("btnconnect").innerText = L.connect;
    //overlayของ Add Email
    document.getElementById("txtAddemail").innerText = L.addEmail;
    document.getElementById("email1").placeholder = L.enteremail1;
    document.getElementById("email2").placeholder = L.enteremail2;
    document.getElementById("email3").placeholder = L.enteremail3;
    document.getElementById("email4").placeholder = L.enteremail4;
    document.getElementById("btnokaddemail").innerText = L.okaddemail;
    document.getElementById("btncanceladdemail").innerText = L.cancelAddemail;
    //overlayของ ReName Device
    document.getElementById("txtRename").innerText = L.renameDevice;
    document.getElementById("newDeviceName").placeholder = L.enterName;
    document.getElementById("btnokrename").innerText = L.okrename;
    document.getElementById("btncancelrename").innerText = L.cancelrename;
    //overlayของ Repassword
    document.getElementById("txtRepass").innerText = L.repassword;
    document.getElementById("newPassword").placeholder = L.enterPass;
    document.getElementById("btnokrepass").innerText = L.okrepass;
    document.getElementById("btncancelrepass").innerText = L.cancelrepass;
    //overlayของ reset
    document.getElementById("txtReset").innerText = L.resetDevice;
    document.getElementById("txtreset").innerText = L.confirmReset;
    document.getElementById("btnokreset").innerText = L.okreset;
    document.getElementById("btncancelreset").innerText = L.cancelreset;
  };  

    </script>
    <script>
    function showAlert(msg){
  var existing = document.getElementById("alertOverlay");
  if(existing) document.body.removeChild(existing);   
  var overlay = document.createElement("div");
  overlay.id = "alertOverlay";
  overlay.style.position = "fixed";
  overlay.style.top = "0";
  overlay.style.left = "0";
  overlay.style.width = "100%";
  overlay.style.height = "100%";
  overlay.style.background = "rgba(0,0,0,0.5)";
  overlay.style.zIndex = "99999";
  overlay.style.display = "flex";
  overlay.style.alignItems = "flex-start";
  overlay.style.paddingTop = "550px";
  overlay.style.justifyContent = "center";

  var box = document.createElement("div");
  box.style.background = "white";
  box.style.borderRadius = "12px";
  box.style.padding = "40px";
  box.style.textAlign = "center";
  box.style.width = "80%";
  box.style.maxWidth = "350px";
  box.style.boxShadow = "0 4px 20px rgba(0,0,0,0.5)";

  box.innerHTML = `
    <p style="font-size:20px;margin-bottom:30px;">${msg}</p>

    <div style = "text-align:right;">
    <button class="btn-primary btn-small"
    onclick="document.body.removeChild(document.getElementById('alertOverlay'))">
    OK
    </button>
    </div>
  `;

  overlay.appendChild(box);
  document.body.appendChild(overlay);
}  
    </script>

    <script>   
  function showSuccess(msg1, msg2){
  var overlay = document.createElement("div");
  overlay.style.position = "fixed";
  overlay.style.top = "0";
  overlay.style.left = "0";
  overlay.style.width = "100%";
  overlay.style.height = "100%";
  overlay.style.background = "rgba(0,0,0,0.5)";
  overlay.style.zIndex = "99999";
  overlay.style.display = "flex";
  overlay.style.alignItems = "flex-start";
  overlay.style.paddingTop = "480px";
  overlay.style.justifyContent = "center";

  var box = document.createElement("div");
  box.style.cssText = "background:white;border-radius:12px;padding:80px;text-align:center;width:90%;max-width:450px;box-shadow:0 4px 20px rgba(0,0,0,0.5);";
  box.innerHTML = `
    <div style="font-size:60px;margin-bottom:20px;">✅</div>
    <p style="font-size:24px;font-weight:bold;margin:0;">${msg1}</p>
    <p style="font-size:20px;color:#666;margin-top:10px;">${msg2}</p>
  `;
  overlay.appendChild(box);
  document.body.appendChild(overlay);
}
    </script>

    <script>
    function showLoading(msg){
      var existing = document.getElementById("alertOverlay");
      if(existing) document.body.removeChild(existing);
      var overlay = document.createElement("div");
      overlay.id = "alertOverlay";
      overlay.style.position = "fixed";
      overlay.style.top = "0";
      overlay.style.left = "0";
      overlay.style.width = "100%";
      overlay.style.height = "100%";
      overlay.style.background = "rgba(0,0,0,0.5)";
      overlay.style.zIndex = "99999";
      overlay.style.display="flex";
      overlay.style.alignItems = "flex-start";
      overlay.style.paddingTop = "550px";
      overlay.style.justifyContent = "center";
      
      var box = document.createElement("div");
      box.style.background = "white";
      box.style.borderRadius = "12px";
      box.style.padding = "40px";
      box.style.textAlign = "center";
      box.style.width = "80%";
      box.style.maxWidth = "350px"; 
      box.style.boxShadow = "0 4px 20px rgba(0,0,0,0.5)";
      box.innerHTML = `<p style="font-size:20px;">${msg}</p>`;
      overlay.appendChild(box);
      document.body.appendChild(overlay);
    }
    </script>

    <p id="netStatus" style="
    position: fixed;
    top: 0px;
    left: 10px;
    color:red;
    font-weight:bold;
    font-size: 20px;
    z-index: 9999;
    ">
      
    </p>

    <script>
    function checkInternetStatus(){
      fetch("/status")
      .then(res => res.text())
      .then(status => {
        let label = document.getElementById("netStatus");
        if(status === "connected"){
          label.innerText = L.statusinterneton;
          label.style.color ="green";
        }
        else{
          label.innerText = L.statusinternetoff;
          label.style.color = "red";
        }
      })
    }
    checkInternetStatus();
    setInterval(checkInternetStatus,3000);
    </script>



    <br>
    <br>
    <br>
    <br>
    <br>
    <br>

    <h1 id="txtWelcome"></h1> 
    <h1 id="txtSetting"></h1>
    <h1 id= "deviceName" style="margin-top:50px;">Name:
    )HTML";
  html += ssid;
  html += R"HTML(
      </h1>
        <!-- Modalต่างๆ -->
    <div id="wifiModal" class="modal">
    <div class ="modal-content">
    <h2 id="txtconnectWifi"></h2>
    <input id="wifi_ssid" type="text">
    <input id="wifi_pass" type="password">
    <br>
    <button id="btncancel" class="btn-primary btn-small" onclick="closeWiFi()"></button>
    <button id="btnconnect"class="btn-primary btn-small" onclick="submitWiFi()"></button>
    </div>
    </div>

    <div id="emailModal" class="modal">
    <div class="modal-content">
    <h2 id="txtAddemail"></h2>
    <input id="email1" type="email">
    <input id="email2" type="email">
    <input id="email3" type="email">
    <input id="email4" type="email">
    <br>
    <button id="btncanceladdemail"class="btn-primary btn-small" onclick="closeEmail()"></button> 
    <button id="btnokaddemail"class="btn-primary btn-small" onclick="saveEmail()"></button>
    </div>
    </div>
    
    <div id="renameModal" class="modal">
    <div class="modal-content">
    <h2 id="txtRename"></h2>
    <input id="newDeviceName" type="text">
    <br>
    <button id="btncancelrename"class="btn-primary btn-small" onclick="closeRename()"></button>
    <button id="btnokrename"class="btn-primary btn-small" onclick="saveRename()"></button>
    </div>
    </div>

    <div id="repasswordModal" class="modal">
    <div class="modal-content">
    <h2 id="txtRepass"></h2>
    <input id="newPassword" type="password">
    <br>
    <button id="btncancelrepass"class="btn-primary btn-small" onclick="closePassword()"></button>
    <button id="btnokrepass"class="btn-primary btn-small" onclick="savePassword()"></button>
    </div>
    </div>

    <div id="resetModal" class="modal">
    <div class="modal-content">
    <h2 id="txtReset"></h2>
    <p id="txtreset" style="font-size:20px;margin:20px 0;">

     </p>
     <button id="btncancelreset"class="btn-primary btn-small" onclick="closeReset()"></button>
     <button id="btnokreset"class="btn-primary btn-small" onclick="confirmReset()"></button>
     </div>
     </div>



      <!-- ปุ่มและฟังก์ชันการทำงานในส่วนต่างๆ -->
    <div class= 'form-box-welcome'>
    <button id="btnconnectWifi" class = "btn-primary btn-large"onclick="openWiFi()"></button>
    <script>
    function openWiFi(){
        fetch("/getwifi")
        .then(res => res.json())
        .then(data => {
          document.getElementById("wifi_ssid").value = data.ssid || "";
          document.getElementById("wifi_pass").value = data.pass || "";
        });
      document.getElementById("wifiModal").style.display = "block";      
    }
    function closeWiFi(){
      document.getElementById("wifiModal").style.display ="none";
    }
    function submitWiFi(){
      var ssid = document.getElementById("wifi_ssid").value;
      var pass = document.getElementById("wifi_pass").value;
     
      if(!ssid){
        showAlert(L.enterSSID);
        return;
      }

       if(pass.length < 8){
        showAlert(L.passShort);
        return;
      }

        var wifiloadingText = (LANG === 'T')? "กำลังเชื่อมต่ออินเทอร์เน็ต โปรดรอสักครู่..." : "Connecting to the internet Please wait...";
          showLoading(wifiloadingText);

      fetch("/connectwifi",{
        method:"POST",
        headers:{
          "Content-Type":"application/x-www-form-urlencoded"
        },
        body:"ssid="+encodeURIComponent(ssid)+"&password="+encodeURIComponent(pass)
      })
      .then(res => res.text())
      .then(msg =>{
         showAlert(msg);
        closeWiFi();
      });
    }
    </script>
    <button id="btnAddemail"class = "btn-primary btn-large"onclick="openEmail()"></button>
    <script>
    function openEmail(){
      fetch("/getemail")
      .then(res => res.json())
      .then(data =>{
        document.getElementById("email1").value = data.e1 || "";
        document.getElementById("email2").value = data.e2 || "";
        document.getElementById("email3").value = data.e3 || "";
        document.getElementById("email4").value = data.e4 || "";
      });
      document.getElementById("emailModal").style.display = "block";
    }
    function closeEmail(){
      document.getElementById("emailModal").style.display = "none";
    }
    
    function isValidEmail(e){
    return /^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(e);
    }
      
    function saveEmail(){
       
      var e1 = document.getElementById("email1").value;
      var e2 = document.getElementById("email2").value;
      var e3 = document.getElementById("email3").value;
      var e4 = document.getElementById("email4").value;
         
         if(!e1 && !e2 && !e3 && !e4){
          showAlert(L.pleaseenteremail);
          return;
          }
      
      
      
         var emails = [e1,e2,e3,e4].filter(e=>e);
         for(let e of emails){
            if(!isValidEmail(e)){
            showAlert(L.invalidemail);
            return;
          } 
         } 
          var loadingText = (LANG === 'T')? "รอระบบบันทึกสักครู่..." : "Please wait...";
          showLoading(loadingText);

          fetch("/saveemail",{
            method:"POST",
            headers:{  "Content-Type":"application/x-www-form-urlencoded"},
            body:"e1="+encodeURIComponent(e1)+
                  "&e2="+encodeURIComponent(e2)+
                  "&e3="+encodeURIComponent(e3)+
                  "&e4="+encodeURIComponent(e4)
          }).then(() =>{
             showAlert(L.saveemaildone);
             closeEmail();
        });   
     }
    </script>
    <button id="btnRename"class = "btn-primary btn-large"onclick="renamesenser()"></button>
    <script>
    function renamesenser(){
       document.getElementById("renameModal").style.display = "block";
      }
      function closeRename(){
        document.getElementById("renameModal").style.display ="none";
      }
      function saveRename(){
   var name = document.getElementById("newDeviceName").value;

    if(!name){
    showAlert(L.pleaseentername);
    return;
  }

    fetch("/rename",{
    method:"POST",
    headers:{
      "Content-Type":"application/x-www-form-urlencoded"
    },
    body:"name="+encodeURIComponent(name)
   }).then(()=>{
    
    showSuccess(L.savenamedone,L.rebooting);
    closeRename();
    setTimeout(()=>location.reload(),2000);
  });
  }
    </script>

    <button id="btnRepass"class = 'btn-primary btn-large'onclick="repassword()"></button>
    <script>
    function repassword(){
      document.getElementById("repasswordModal").style.display ="block";
    }
    function closePassword(){
      document.getElementById("repasswordModal").style.display = "none";
    }
    function savePassword(){
      var pass = document.getElementById("newPassword").value;
      if(!pass){
         showAlert(L.pleaseenterpass);
        return;
      }
      if(pass.length < 8){
         showAlert(L.passShort);
        return;
      }
      fetch("/repassword",{
        method:"POST",
        headers:{
          "Content-Type":"application/x-www-form-urlencoded"
        },
        body:"password="+encodeURIComponent(pass)
      }).then(()=>{

       showSuccess(L.savepassdone,L.rebooting);
        closePassword();
        setTimeout(()=>location.reload(),2000);
      });
    }
    </script>

    

    <button id="btnReset" class = 'btn-primary btn-large' onclick="openReset()"></button>
    <script>
    function openReset(){
      document.getElementById("resetModal").style.display = "block";
    }
    function closeReset(){
      document.getElementById("resetModal").style.display ="none";
    }
    function confirmReset(){
      
      fetch("/reset",{method:"POST"})
      .then(()=>{
         closeReset(); 
        showSuccess(L.resetDone,L.rebooting);
        
      });
      }
    
    </script>
 

    </div>

   
  )HTML";


  html += "<script>var DEVICE_ID = '" + deviceId + "';</script>";
  html += pageFooter;

  server.send(200, "text/html", html);
}

// ฟังก์ชันบันทึก wifi  และ password
void saveWiFicred(String ssid, String pass) {
  ssid = ssid.substring(0, 32);
  pass = pass.substring(0, 32);
  for (int i = 0; i < 32; i++) {
    EEPROM.write(WiFi_SSID_ADDR + i, i < ssid.length() ? ssid[i] : 0);
    EEPROM.write(WIFI_PASS_ADDR + i, i < pass.length() ? pass[i] : 0);
  }
  EEPROM.commit();
}
// โหลด ssid ของ wifi
String loadWiFiSSID() {
  char buf[33];
  for (int i = 0; i < 32; i++) {
    char c = EEPROM.read(WiFi_SSID_ADDR + i);
    if (c == 0xff) c = 0;
    buf[i] = c;
  }
  buf[32] = 0;
  return String(buf);
}
// ฟังก์ชัน โหลด wifipassword
String loadWiFiPASS() {
  char buf[33];
  for (int i = 0; i < 32; i++) {
    char c = EEPROM.read(WIFI_PASS_ADDR + i);
    if (c == 0xFF) c = 0;
    buf[i] = c;
  }
  buf[32] = 0;
  return String(buf);
}
//ฟังก์ชันนำ ssidwifi ทีบันทึกใน EEPROM  นำมาแสดง
String loadshowssidwifi(int addr) {
  char buf[33];
  for (int i = 0; i < 32; i++) {
    char c = EEPROM.read(addr + i);
    if (c == 0xFF) c = 0;
    buf[i] = c;
  }
  buf[32] = 0;
  return String(buf);
}
//ฟังก์ชันนำ passwordwifi ทีบันทึกใน EEPROM  นำมาแสดง
String loadshowpasswordwifi(int addr) {
  char buf[33];
  for (int i = 0; i < 32; i++) {
    char c = EEPROM.read(addr + i);
    if (c == 0xFF) c = 0;
    buf[i] = c;
  }
  buf[32] = 0;
  return String(buf);
}


// ฟังชันก์บันทึกชื่อ
void saveSSID(String name) {
  if (name.length() > 32) {
    name = name.substring(0, 32);
  }
  for (int i = 0; i < 32; i++) {
    if (i < name.length()) {
      EEPROM.write(SSID_ADDR + i, name[i]);
    } else {
      EEPROM.write(SSID_ADDR + i, 0);
    }
  }
  EEPROM.commit();
}

String loadSSID() {
  char buf[33];
  for (int i = 0; i < 32; i++) {
    char c = EEPROM.read(SSID_ADDR + i);
    if (c == 0xFF) c = 0;
    buf[i] = c;
  }
  buf[32] = '\0';
  if (strlen(buf) == 0) return defaultssid;
  return String(buf);
}
//ฟังก์ชันบันทึกรหัสผ่าน
void savePassword(String pass) {
  if (pass.length() < 8) return;  //wifi ต้องมี>= 8 ตัว
  if (pass.length() > 32) pass = pass.substring(0, 32);
  for (int i = 0; i < 32; i++) {
    if (i < pass.length())
      EEPROM.write(PASS_ADDR + i, pass[i]);
    else
      EEPROM.write(PASS_ADDR + i, 0);
  }
  EEPROM.commit();
}

//ฟังชันก์โหลด Password จาก EEPROM
String loadPassword() {
  char buf[33];
  for (int i = 0; i < 32; i++) {
    char c = EEPROM.read(PASS_ADDR + i);
    if (c == 0xFF) c = 0;
    buf[i] = c;
  }
  buf[32] = '\0';
  if (strlen(buf) < 8) return defaultpass;  // ค่าเริ่มต้น
  return String(buf);
}

//ฟังก์ชันส่ง Email ไปยัง server
void sendEmailFromESP(String email[], int count, String deviceName, String senser) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected");
    return;
  }
  HTTPClient http;
  http.begin("https://pir-senser-backend.onrender.com/send-email");
  http.addHeader("Content-Type", "application/json");
  String body = "{";
  body += "\"email\":[";
  for (int i = 0; i < count; i++) {
    if (i > 0) body += ",";
    body += "\"" + email[i] + "\"";
  }
  body += "],";
  body += "\"deviceName\":\"" + deviceName + "\",";
  body += "\"lang\":\"" + String(currentLang) + "\",";
  body += "\"senser\":\"" + senser + "\"";
  body += "}";
  int httpCode = http.POST(body);
  String payload = http.getString();
  if (httpCode > 0) {
    Serial.print("Email sent, count:" + String(count));
    Serial.print("Status:" + String(httpCode));
    Serial.println("Response:" + payload);
  } else {
    Serial.println("Email failed");
  }
  http.end();
}
//

//ฟังก์ชันsave email
void saveEmail(int addr, String email) {
  email = email.substring(0, 32);
  for (int i = 0; i < 32; i++) {
    EEPROM.write(addr + i, i < email.length() ? email[i] : 0);
  }
  EEPROM.commit();
}

// ฟังก์ชันนำ Email เพื่อไปแสดง
String loadEmail(int addr) {
  char buf[33];
  for (int i = 0; i < 32; i++) {
    char c = EEPROM.read(addr + i);
    if (c == 0xFF) c = 0;
    buf[i] = c;
  }
  buf[32] = 0;
  return String(buf);
}

//ฟังก์ชันล้างค่าข้อมูล
void factoryReset() {
  for (int i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 0);
  }
  for (int i = WiFi_SSID_ADDR; i < WIFI_PASS_ADDR + 32; i++) {
    EEPROM.write(i, 0);
  }


  for (int i = 0; i < defaultssid.length(); i++) {
    EEPROM.write(SSID_ADDR + i, defaultssid[i]);
  }

  for (int i = 0; i < defaultpass.length(); i++) {
    EEPROM.write(PASS_ADDR + i, defaultpass[i]);
  }

  for (int i = EMAIL1_ADDR; i < EMAIL4_ADDR + 32; i++) {
    EEPROM.write(i, 0);
  }

  EEPROM.commit();
  Serial.println("Factory reset done");
  delay(500);
  ESP.restart();
}
// ฟังก์ชัน pirsenser
void checkPIR() {
  int val = digitalRead(pirsenser_pin);
  if (val == 1) {

    unsigned long now = millis();
    if (now - lastEmailTime > emailCooldown) {
      lastEmailTime = now;
      Serial.println("Motion detected!");
      // ส่ง email ทีบันทึกไว้
      String emails[4];
      int count = 0;
      int addrList[] = { EMAIL1_ADDR, EMAIL2_ADDR, EMAIL3_ADDR, EMAIL4_ADDR };
      for (int i = 0; i < 4; i++) {
        String e = loadEmail(addrList[i]);
        if (e.length() > 5) {
          emails[count] = e;
          count++;
        }
      }
      if (count > 0) {
        sendEmailFromESP(emails, count, ssid, "ALERT");
      }
    }
  }
}
void setup() {
  pinMode(pirsenser_pin, INPUT);
  Serial.begin(115200);

  const esp_task_wdt_config_t wdt_config = {
    .timeout_ms = 10000,
    .idle_core_mask = 0,
    .trigger_panic = true
  };
  esp_task_wdt_init(&wdt_config);
  esp_task_wdt_add(NULL);
  server.on("/generate_204", handleLogin);         //Android
  server.on("/hotspot-detect.html", handleLogin);  //ios
  server.on("/fwlink", handleLogin);               //Windows
  EEPROM.begin(EEPROM_SIZE);
  // eeprom ของภาษา
  savedLang = EEPROM.read(LANG_ADDR);
  if (savedLang == 'T' || savedLang == 'E') {
    currentLang = savedLang;
  }
  Serial.println("WiFi Password Loaded");
  WiFi.mode(WIFI_AP_STA);
  deviceId = WiFi.macAddress();
  Serial.println("Device ID:" + deviceId);


  ssid = loadSSID();
  apPassword = loadPassword();
  WiFi.softAP(ssid.c_str(), apPassword.c_str());
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
  String sta_ssid = loadWiFiSSID();
  String sta_pass = loadWiFiPASS();
  if (sta_ssid.length() > 0) {
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(sta_ssid.c_str(), sta_pass.c_str());
    Serial.print("Auto connecting to WiFi");
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 20000)  //ค่าเดิม10000
    {
      delay(500);
      Serial.print(".");
      esp_task_wdt_reset();
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\n Auto WiFi Connected");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("\n Auto WiFi Failed");
    }
  }
  server.on("/lang/th", []() {
    currentLang = 'T';
    EEPROM.write(LANG_ADDR, 'T');
    EEPROM.commit();
    Serial.println("Language: TH");
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });

  server.on("/lang/en", []() {
    currentLang = 'E';
    EEPROM.write(LANG_ADDR, 'E');
    EEPROM.commit();
    Serial.println("Language: EN");
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });
  server.on("/", handleLogin);  //"/login"
  //เชื่อมต่อwifi
  server.on("/connectwifi", HTTP_POST, []() {
    String wifi_ssid = server.arg("ssid");
    String wifi_pass = server.arg("password");

    Serial.println("Connecting to WiFi...");
    Serial.println("SSID:" + wifi_ssid);
    Serial.println("PASS:" + wifi_pass);

    WiFi.mode(WIFI_AP_STA);
    WiFi.disconnect(true);
    delay(500);
    saveWiFicred(wifi_ssid, wifi_pass);
    WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());


    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
      delay(500);
      Serial.print(".");
      yield();
      esp_task_wdt_reset();
    }



    if (currentLang == 'E') {
      lang1 = "✅ Connected to Internet!";
      lang0 = "❌ Connection Failed";
    } else {
      lang1 = "✅ เชื่อมต่อกับอินเทอร์เน็ตได้แล้ว!";
      lang0 = "❌ การเชื่อมต่อล้มเหลว";
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi Connected!");
      Serial.println(WiFi.localIP());
      server.send(200, "text/plain", lang1);
    } else {
      Serial.println("\nWiFi Failed!");
      server.send(200, "text/plain", lang0);
    }
  });
  server.on("/status", HTTP_GET, []() {
    if (WiFi.status() == WL_CONNECTED) {
      server.send(200, "text/plain", "connected");
    } else {
      server.send(200, "text/plain", "not_connected");
    }
  });
  //นำ ssid และ password ที่เก็บใว้ใน EEPROM นำมาแสดง
  server.on("/getwifi", HTTP_GET, []() {
    String json = "{";
    json += "\"ssid\":\"" + loadshowssidwifi(WiFi_SSID_ADDR) + "\",";
    json += "\"pass\":\"" + loadshowpasswordwifi(WIFI_PASS_ADDR) + "\" ";
    json += "}";
    server.send(200, "application/json", json);
  });

  server.on("/saveemail", HTTP_POST, []() {
    String e1 = server.arg("e1");
    String e2 = server.arg("e2");
    String e3 = server.arg("e3");
    String e4 = server.arg("e4");

    saveEmail(EMAIL1_ADDR, e1);
    saveEmail(EMAIL2_ADDR, e2);
    saveEmail(EMAIL3_ADDR, e3);
    saveEmail(EMAIL4_ADDR, e4);

    Serial.println("Email1: " + e1);
    Serial.println("Email2: " + e2);
    Serial.println("Email3: " + e3);
    Serial.println("Email4: " + e4);

    server.send(200, "text/plain", "OK");

    String emails[4];
    int count = 0;
    if (e1.length() > 5) emails[count++] = e1;
    if (e2.length() > 5) emails[count++] = e2;
    if (e3.length() > 5) emails[count++] = e3;
    if (e4.length() > 5) emails[count++] = e4;
    if (count > 0) sendEmailFromESP(emails, count, ssid, "SAVE");
  });
  //นำ Email ที่เคยพิมพ์มาก่อนนำมาแสดง
  server.on("/getemail", HTTP_GET, []() {
    String json = "{";
    json += "\"e1\":\"" + loadEmail(EMAIL1_ADDR) + "\",";
    json += "\"e2\":\"" + loadEmail(EMAIL2_ADDR) + "\",";
    json += "\"e3\":\"" + loadEmail(EMAIL3_ADDR) + "\",";
    json += "\"e4\":\"" + loadEmail(EMAIL4_ADDR) + "\" ";
    json += "}";
    server.send(200, "application/json", json);
  });

  server.on("/rename", HTTP_POST, []() {
    String newName = server.arg("name");
    if (newName.length() > 0) {
      saveSSID(newName);
      server.send(200, "text/plain", "OK");
      delay(500);
      ESP.restart();
    } else {
      server.send(400, "text/plain", "ERROR");
    }
  });

  server.on("/repassword", HTTP_POST, []() {
    String pass = server.arg("password");
    if (pass.length() >= 8) {
      savePassword(pass);
      server.send(200, "text/plain", "OK");
      delay(500);
      ESP.restart();
    } else {
      server.send(400, "text/plain", "PASSWORD_TOO_SHORT");
    }
  });

  server.on("/reset", HTTP_POST, []() {
    server.send(200, "text/plain", "RESET");
    delay(300);
    factoryReset();
  });


  server.begin();
  Serial.println("AP SSID: " + ssid);
  Serial.println("Server started at IP:" + WiFi.softAPIP().toString());
}
void loop() {
  esp_task_wdt_reset();
  dnsServer.processNextRequest();
  server.handleClient();
  checkPIR();
}