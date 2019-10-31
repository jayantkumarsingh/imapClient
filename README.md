# imap client for gmail
## sms using twilio (www.twilio.com)

### Build and Run environment
MinGW for Windows
Deps: libCurl
Build: makefile
IDE: Sublime Text 3

#### Description
This is a sample program to continuosly poll gmail inbox for any new mail.
If a mail arrives, the body and header is copied in local buffers. Size of the buffers should be customizable as per need.
Additionally, the a notification is send as sms using Twilio.
