/** 
 * @file Networking.cpp
 * @brief Contains networking utility functions.
 * @author Igor Janjic
 * @date 01/31/2018
 *
 * Updates:
 * 01/31/2018 [janjic.igor] Created file
 */

#include "Networking.h"


uint64_t Networking::strToUint(const std::string& str)
{
    std::istringstream i(str);
    uint64_t value;
    if (!(i >> value))
        value = 0;
    return value;
}


int64_t Networking::strToInt(const std::string& str)
{
    std::istringstream i(str);
    int64_t value;
    if (!(i >> value))
        value = 0;
    return value;
}


double Networking::strToFloat(const std::string& str)
{
    std::istringstream i(str);
    double value;
    if (!(i >> value))
        value = 0;
    return value;
}


uint32_t Networking::bytes2Uint(const char bytes[4])
{
    return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
}


double Networking::getWallTime()
{
    struct timeval t;
    if (gettimeofday(&t, NULL))
        return 0;
    return (double)t.tv_sec + (double)t.tv_usec * .000001;
}


bool Networking::sendData(int fd, const std::string& data)
{
    return Networking::sendData(fd, data.c_str(), data.length());
}


bool Networking::sendData(int fd, const char* data, int length)
{
    if (::send(fd, data, length, 0) < 0)
    {
        std::cerr << "Send failed" << std::endl; 
        return false;
    }
    return true;
}


bool Networking::receiveData(std::string& data, int& bytes, int fd, int size)
{
    char buffer[size + 1];

    if (!Networking::receiveData(buffer, bytes, fd, size))
        return false;

    buffer[size] = '\0';
    data = buffer;
    return true;
}


bool Networking::receiveData(char* data, int& bytes, int fd, int size)
{
    memset(data, 0, sizeof(char)*size);

    bytes = ::recv(fd, data, size, 0);
    if (bytes == -1)
        return false;
    return true;
}


bool Networking::read(char* data, int& bytes, int fd, int size)
{
    bytes = 0;
    while (bytes != size)
    {
        int curBytes = 0;
        if (!Networking::receiveData(data + bytes, curBytes, fd, size - bytes))
        {
            if (errno == EAGAIN)
                continue;      
            else
                return false;
        }

        bytes += curBytes;
    }
    return true;
}


bool Networking::hasInput(int fd, double timeout)
{
    int status;
    fd_set fds;
    struct timeval tv;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    tv.tv_sec  = (long)timeout;
    tv.tv_usec = (long)((timeout - tv.tv_sec) * 1000000);

    while (true)
    {
        if (!(status = select(fd + 1, &fds, 0, 0, &tv)))
            return false;
        else if (status > 0 && FD_ISSET(fd, &fds))
            return true;
        else if (status > 0)
            return false;
        else if (errno != EINTR)
            return false;
    }
}


bool Networking::flushSocket(int fd, double timeout)
{
   const double start = Networking::getWallTime();
   char discard[100];

   bool hasInput = false;
   if (shutdown(fd, SHUT_WR) != -1)
   {
        while (Networking::getWallTime() < start + timeout)
        {
            while (hasInput = Networking::hasInput(fd, 0.01))
                if (!::read(fd, discard, sizeof(discard)))
                    return true;

            if (!hasInput)
                return true;
        }
   }
   return false;
}


bool Networking::setTimeoutReceive(int fd, double timeout)
{
    struct timeval tv;
    tv.tv_sec  = (long)timeout;
    tv.tv_usec = (long)((timeout - tv.tv_sec) * 1000000);
    int retval = ::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));
    return retval == -1 ? false : true;
}


bool Networking::setTimeoutSend(int fd, double timeout)
{
    struct timeval tv;
    tv.tv_sec  = (long)timeout;
    tv.tv_usec = (long)((timeout - tv.tv_sec) * 1000000);
    int retval = setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&tv, sizeof(tv));
    return retval == -1 ? false : true;
}


bool Networking::setTimeout(int fd, double timeoutSend, double timeoutReceive)
{
    return Networking::setTimeoutReceive(fd, timeoutReceive)
        && Networking::setTimeoutSend(fd, timeoutSend);
}


bool Networking::getSocketState(int& error, int fd)
{
    socklen_t len = sizeof(error);
    int retval = ::getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len);
    return retval == -1 ? false : true;
}


bool Networking::getSocketState(int fd)
{
    int error;
    return Networking::getSocketState(error, fd);
}

