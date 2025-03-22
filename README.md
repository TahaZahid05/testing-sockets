This is a guide on how to get everthing up and running

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
    
    on multiple devices connected to same network connection for better understanding.