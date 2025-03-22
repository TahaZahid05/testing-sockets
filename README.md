<!-- This is a guide on how to get everthing up and running

1) Installing Qt

    a) Run "qt-online-installer-windows-x64-4.8.1"

    b) Make a account of qt (either in the app or online)

    c) Agree to the conditions

    d) Keep going Next

    e) Choose "Custom Installation" and tick "Associate common file types with Qt Creator"

    f) Search for "websockets" and select "Qt WebSockets" under "Qt 6.8.2"

    g) Keep going next and install Qt

2) Open Qt Creator

    a) Click on "Open Project"

    b) Go into our github repository and select "chatapp.pro" (Will appear as chatapp with Type as "Qt Project File"), and click on 

    "Open"

    c) Before doing anything else, go into your repository and open "chatwindow.cpp". On Line 21, we have to change IPv4 address. Press the windows key, search cmd and "Run as administrator". Type "ipconfig", and look for IPv4 address, it should look like "192.168.x.xxx", copy this address and replace it in line 21.

    d) Back into Qt Creator, In top-left, click Build and then Build All Projects

    e) Click bottom right "Build" to keep track of the build

    f) If it show "Elapsed time", build has been completed

    g) In github repository, open "server.py" which is in folder "server". Run this python script
    
    h) In Qt Creator, in top-left, Click Build -> Run. Your program would open up and in "server.py" output terminal, you would see
    
    "WebSocket Server Started on ws://0.0.0.0:12345
    
    New client connected. Total clients: 1" 

    i) Enter your "hello" in the lineEdit bar and then click on the button. Your text will now appear in the TextEdit and in 
    
    "server.py" terminal you would see "Received: hello". Server is receiving your input and writing into TextEdit. You can test it
    
    on multiple devices connected to same network connection for better understanding. -->

    # ChatApp Setup Guide 🚀  

Follow this guide to install and run the **ChatApp** using **Qt** and **Python WebSockets**.  

---

## 📌 1. Installing Qt  

1. Run **`qt-online-installer-windows-x64-4.8.1`**.  
2. Create a **Qt account** (either in the app or online).  
3. Accept the **terms and conditions**.  
4. Keep clicking **Next** until the installation options appear.  
5. Select **"Custom Installation"** and tick **"Associate common file types with Qt Creator"**.  
6. Search for **"websockets"** and select **"Qt WebSockets"** under **"Qt 6.8.2"**.  
7. Click **Next** and install Qt.  

---

## 📌 2. Opening the Project in Qt Creator  

1. Open **Qt Creator**.  
2. Click **"Open Project"**.  
3. Navigate to the **GitHub repository**, select **`chatapp.pro`** (Type: *Qt Project File*), and click **"Open"**.  
4. **Update the IPv4 address**:  
   - Open **`chatwindow.cpp`** from the repository.  
   - Press **Windows Key + Search for "cmd"**, then **"Run as Administrator"**.  
   - Type:  
     ```sh
     ipconfig
     ```
   - Look for the **IPv4 address** (e.g., `192.168.x.xxx`) and **replace it in Line 21 of `chatwindow.cpp`**.  

---

## 📌 3. Building and Running the Application  

1. In **Qt Creator**, click **Build → Build All Projects**.  
2. Click **"Build"** (bottom-right) to monitor the build process.  
3. If you see **"Elapsed time"**, the build is complete.  

---

## 📌 4. Running the Server  

1. In the **GitHub repository**, navigate to the **"server"** folder.  
2. Open `server.py` and run:  
   ```sh
   python server.py
