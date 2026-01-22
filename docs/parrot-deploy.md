Instructions for deploying the ASL Parrot on AWS. These instructions
are based on a manual deployment. Things will certainly be different for 
a more automated process.

# Some Assumptions I'm Making

* This is a low-volume, low-criticality application. There is no need to be doing
load balancing across redundant copies, etc.
* A single EC2 instance will be used.
* Bruce will be able to redeploy the application independently until it is stable.

# Things You Need To Deploy

These instructions assume you are starting from nothing except:
* The URL of the .deb packages (get that from Bruce)
* The ASL node number and password for the parrot node 
* An AWS account with permissions to create an EC2 instance.

# Steps To Install

Network setup:
* Put the instance in a VPC/subnet that has IPv6 addressing enabled.
* Put the instance on a subnet that has an IPv6 route to the internet.
* Create two Security Groups, one that allows IAX/SSH inbound and the
other that is outbound only (for network diagnostics)
* Create two network interfaces, assigned to the two security groups 
respectively
* Create two elastic IP addresses, one for normal IAX/SSH activity and 
one for network diagnostics.

Create an EC2 instance:
* Debian 13, arm-64 using t4g-small instance type (t4g-micro being tested)
* If you don't already have a keypair, create one called "parrot-1"
* If necessary, download the private half of the keypair to ~/.ssh/parrot-1.pem so that you can log in using SSH.
* Accept the default EBS size of 8G.
* Wait for the instance to come up.

If not created previously, get the public IPv4 address from the EC2 console. Use SSH to log into the new instance as admin:

    ssh -i ~/.ssh/parrot-1.pem admin@13.57.189.175

(The rest of the steps are executed on the new EC2 instance,
all from the admin home directory.)

Add the required Linux packages:

    sudo apt update
    sudo apt -y upgrade
    sudo apt -y install net-tools build-essential gdb cmake git emacs-nox python3.13-venv wget 

Get the .deb file:

    wget https://mackinnon.info/ampersand/releases/asl-parrot_1.4-1_arm64.deb

Install the package:

    sudo apt install ./asl-parrot_1.4-1_arm64.deb

NOTE: There may be a notice displayed that contains "permission denied." If this is 
just a notice it can be ignored.

**Before starting the service** make adjustments /usr/etc/asl-parrot.env file.

Add the secrets here:

    AMP_NODE0_NUMBER=nnnnn
    AMP_NODE0_PASSWORD=xxxx

Change the bind address used for network diagnostic testing to the INTERNAL
IP address associated with the network interface that was setup for network
diagnostics.

    # Network interface (IPv4) that is used to initiate network tests.
    # Needs to be different from the interface accepting IAX connections
    # for the test to be fully effective
    AMP_NET_TEST_BIND_ADDR4=172.31.23.91

Adjust the HTTP listen port (internal VPC only) as needed:

    AMP_HTTP_PORT=8080

And then enable and start the service:

    sudo systemctl enable asl-parrot
    sudo systemctl start asl-parrot

Check the log:

    journalctl -u asl-parrot.service -f

# Network Configuration

![Security Group](sg1.jpg)

# Network Test API 

Example request:
    
    curl asl-parrot:8080/network-test?node=2002

Example response (good case):

    {"ipv4":{"addr":"18.226.187.225","pingms":49,"port":4569,"rc":0,"status":"ok"}}

Example response (bad case):

    {"ipv4":{"rc":-9,"status":"unreachable"}}
