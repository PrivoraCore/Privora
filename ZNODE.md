Znode Build Instructions and Notes
=============================
 - Version 0.1.6
 - Date: 14 December 2017

Prerequisites
-------------
 - Ubuntu 16.04+
 - Libraries to build from Privora source
 - Port **5660** is open

Step 1. Build
----------------------
**1.1.**  Check out from source:

    git clone https://github.com/PrivoraCore/Privora

**1.2.**  See [README.md](README.md) for instructions on building.

Step 2. (Optional - only if firewall is running). Open port 5660
----------------------
**2.1.**  Run:

    sudo ufw allow 5660
    sudo ufw default allow outgoing
    sudo ufw enable

Step 3. First run on your Local Wallet
----------------------
**3.0.**  Go to the checked out folder

    cd privora

**3.1.**  Start daemon in testnet mode:

    ./src/privorad -daemon -server -testnet

**3.2.**  Generate znodeprivkey:

    ./src/privora-cli znode genkey

(Store this key)

**3.3.**  Get wallet address:

    ./src/privora-cli getaccountaddress 0

**3.4.**  Send to received address **exactly 1000 PRIVORA** in **1 transaction**. Wait for 15 confirmations.

**3.5.**  Stop daemon:

    ./src/privora-cli stop

Step 4. In your VPS where you are hosting your Znode. Update config files
----------------------
**4.1.**  Create file **privora.conf** (in folder **~/.privora**)

    rpcuser=username
    rpcpassword=password
    rpcallowip=127.0.0.1
    debug=1
    txindex=1
    daemon=1
    server=1
    listen=1
    maxconnections=24
    znode=1
    znodeprivkey=XXXXXXXXXXXXXXXXX  ## Replace with your znode private key
    externalip=XXX.XXX.XXX.XXX:5660 ## Replace with your node external IP

**4.2.**  Create file **znode.conf** (in 2 folders **~/.privora** and **~/.privora/testnet3**) contains the following info:
 - LABEL: A one word name you make up to call your node (ex. ZN1)
 - IP:PORT: Your znode VPS's IP, and the port is always 15660.
 - ZNODEPRIVKEY: This is the result of your "znode genkey" from earlier.
 - TRANSACTION HASH: The collateral tx. hash from the 1000 PRIVORA deposit.
 - INDEX: The Index is always 0 or 1.

To get TRANSACTION HASH, run:

    ./src/privora-cli znode outputs

The output will look like:

    { "d6fd38868bb8f9958e34d5155437d009b72dfd33fc28874c87fd42e51c0f74fdb" : "0", }

Sample of znode.conf:

    ZN1 51.52.53.54:15660 XrxSr3fXpX3dZcU7CoiFuFWqeHYw83r28btCFfIHqf6zkMp1PZ4 d6fd38868bb8f9958e34d5155437d009b72dfd33fc28874c87fd42e51c0f74fdb 0

Step 5. Run a znode
----------------------
**5.1.**  Start znode:

    ./src/privora-cli znode start-alias <LABEL>

For example:

    ./src/privora-cli znode start-alias ZN1

**5.2.**  To check node status:

    ./src/privora-cli znode debug

If not successfully started, just repeat start command
