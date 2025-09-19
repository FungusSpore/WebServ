# Web Server

A lightweight web server built in C with the help of [EdwinGH-1](https://github.com/EdwinGH-1) and [vlow-dev](https://github.com/vlow-dev).  
This project uses **epoll**, so it only works on **Linux** systems.  

---

## 🚀 Features
- Uses **epoll** for efficient I/O handling  
- Minimal dependencies  
- Config-driven (similar to Nginx)  
- Simple and fast  

---

## ⚠️ Requirements
- Linux (epoll is not available on Windows/Mac natively)  
- GCC or Clang  
- `make` installed  

---

## 🛠️ Build & Run
```bash
# Clone the repository
git clone https://github.com/your-username/your-repo.git
cd your-repo

# Build
make

# Run the server with your config
./server path/to/config.conf
