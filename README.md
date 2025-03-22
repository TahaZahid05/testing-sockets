# Setup Guide  

Follow this guide to install and run the **ChatApp** using **Qt** and **Python WebSockets**.  

---

## 1. Installing Qt  

1. Run **`qt-online-installer-windows-x64-4.8.1`**.  
2. Create a **Qt account** (either in the app or online).  
3. Accept the **terms and conditions**.  
4. Keep clicking **Next** until the installation options appear.  
5. Select **"Custom Installation"** and tick **"Associate common file types with Qt Creator"**.  
6. Search for **"websockets"** and select **"Qt WebSockets"** under **"Qt 6.8.2"**.  
7. Click **Next** and install Qt.  

---

## 2. Opening the Project in Qt Creator  

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

## 3. Building and Running the Application  

1. In **Qt Creator**, click **Build → Build All Projects**.  
2. Click **"Build"** (bottom-right) to monitor the build process.  
3. If you see **"Elapsed time"**, the build is complete.  

---

## 4. Running the Server  

1. In the **GitHub repository**, navigate to the **"server"** folder.  
2. Open `server.py` and run:  
   ```sh
   python server.py
    ```
3. You should see:
    ```sh
    WebSocket Server Started on ws://0.0.0.0:12345
    ```

---

## 5. Running the Chat Application

1. In **Qt Creator**, click **Build -> Run**.
2. The application will open, and the **`server.py`** terminal should display:
    ```sh
    New client connected. Total clients: 1
    ```
3. Test the chat functionality:
    - Enter **`hello`** in the **QLineEdit** bar.
    - Click the **Send Button**.
    - Your text will appear in the **QTextEdit**.
    - The **`server.py`** terminal will show:
      ```sh
      Received: hello
      ```
4. Try connecting from multiple devices on the **same Wi-Fi network for a better understanding.**
