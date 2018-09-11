/** 
 * @file Networking.h
 * @brief Contains networking utility functions.
 * @author Igor Janjic
 * @date 01/31/2018
 *
 * Updates:
 * 01/31/2018 [janjic.igor] Created file
 */

#ifndef NETWORKING_H
#define NETWORKING_H

// STL
#include <cstring>
#include <string>
#include <sstream>
#include <iostream>
#include <unistd.h>

// Time
#include <time.h>
#include <sys/time.h>

// Network
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>


namespace Networking {


/** Gets the current wall time in seconds.
 *
 *  @return             Wall time in seconds.
 */
double getWallTime();


/** Converts a string to a uint.
 *
 *  @param str  String to convert.
 *  @return     Converted value.
 */
uint64_t strToUint(const std::string& str);


/** Converts a string to an int.
 *
 *  @param str  String to convert.
 *  @return     Converted value.
 */
int64_t strToInt(const std::string& str);


/** Converts a string to a double.
 *
 *  @param str  String to convert.
 *  @return     Converted value.
 */
double strToFloat(const std::string& str);


/** Converts a 4 byte char array into an unsigned int.
 */
uint32_t bytes2Uint(const char bytes[4]);


/** Sends data to the server.
 *
 *  @param[in] fd       File descriptor.
 *  @param[in] data     Data to be sent in string form.
 *  @return             True if the data was sent.
 */
bool sendData(int fd, const std::string& data);


/** Sends data to the server.
 *
 *  @param[in] fd       File descriptor.
 *  @param[in] data     Data to be sent in buffer form.
 *  @return             True if the data was sent.
 */
bool sendData(int fd, const char* data, int length);


/** Receives data from the server.
 *
 *  @param[out] data    Data to be received is stored here in string form.
 *  @param[out] bytes   Number of bytes actually received.
 *  @param[in]  fd      File descriptor.
 *  @param[in]  size    Amount of data to read in bytes.
 *  @return             True if the data was received.
 */
bool receiveData(std::string& data, int& bytes, int fd, int size);


/** Receives data from the server.
 *
 *  User must make sure buffer has been allocated to the correct size: size + 1
 *  to hold the end of string character.
 *
 *  @param[out] data    Data to be received is stored here in buffer form.
 *  @param[out] bytes   Number of bytes actually received.
 *  @param[in]  fd      File descriptor.
 *  @param[in]  size    Amount of data to read in bytes.
 *  @return             True if the data was received.
 */
bool receiveData(char* data, int& bytes, int fd, int size);


/** Reads until all bytes have been read.
 *
 *  @param[out] data        Data to be received is stored here in buffer form.
 *  @param[out] bytes       Number of bytes actually received.
 *  @param[in]  fd          File descriptor.
 *  @param[in]  size        Number of bytes to read before stoping.
 *  @return                 True if the data was read.
 */
bool read(char* data, int& bytes, int fd, int size);


/** Wrapper for select(2) which determines if there is any data left on the
 *  socket until the timeout expires.
 *
 *  @param[in]  fd       File descriptor of the socket to close.
 *  @param[in]  timeout  Amount of time to block.
 *  @return              True if has input, otherwise false.
 *
 *  Code credit: Joseph Quinsey @ stackoverflow
 *  https://stackoverflow.com/questions/12730477/close-is-not-closing-socket-properly
 */
bool hasInput(int fd, double timeout);


/** Flushes a socket.
 *  
 *  @param[in] fd       File descriptor of the socket to flush.
 *  @param[in] timeout  Amount of time to block.
 *
 *  Code credit: Joseph Quinsey @ stackoverflow
 *  https://stackoverflow.com/questions/12730477/close-is-not-closing-socket-properly
 */
bool flushSocket(int fd, double timeout);


/** Sets a receive timeout for a socket.
 *
 *  @param timeout      Receive timeout for socket.
 *  @return             True if the timeout was successfully set.
 */
bool setTimeoutReceive(int fd, double timeout);


/** Sets a send timeout for a socket.
 *
 *  @param timeout      Send timeout for socket.
 *  @return             True if the timeout was successfully set.
 */
bool setTimeoutSend(int fd, double timeout);


/** Sets a socket timeout for both send and receive.
 *
 *  @param timeoutSend      Send timeout for socket.
 *  @param timeoutReceive   Receive timeout for socket.
 *  @return                 True if the timeout was successfully set.
 */
bool setTimeout(int fd, double timeoutSend, double timeoutReceive);


/** Gets the current state of the client socket by checking if there is an
 *  error. This function can be used to implement some form of dead socket
 *  detection.
 *
 *  @param[out] error    Error code if there was an error.
 *  @param[in]  fd       File descriptor of the socket to check.
 *  @return              Returns true if there is no error, otherwise false and
 *                       the parameter is set to the error code.
 */
bool getSocketState(int& error, int fd);
bool getSocketState(int fd);


}  // NETWORKING


#endif  // NETWORKING_H

