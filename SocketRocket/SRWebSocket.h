//
// Copyright 2012 Square Inc.
// Portions Copyright (c) 2016-present, Facebook, Inc.
// 
// All rights reserved.
// 
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant 
// of patent rights can be found in the PATENTS file in the same directory.
//

#import <Foundation/Foundation.h>
#import <Security/SecCertificate.h>

typedef NS_ENUM(NSInteger, SRReadyState) {
    SR_CONNECTING   = 0,
    SR_OPEN         = 1,
    SR_CLOSING      = 2,
    SR_CLOSED       = 3,
};

typedef enum SRStatusCode : NSInteger {
    // 0–999: Reserved and not used.
    SRStatusCodeNormal = 1000,
    SRStatusCodeGoingAway = 1001,
    SRStatusCodeProtocolError = 1002,
    SRStatusCodeUnhandledType = 1003,
    // 1004 reserved.
    SRStatusNoStatusReceived = 1005,
    SRStatusCodeAbnormal = 1006,
    SRStatusCodeInvalidUTF8 = 1007,
    SRStatusCodePolicyViolated = 1008,
    SRStatusCodeMessageTooBig = 1009,
    SRStatusCodeMissingExtension = 1010,
    SRStatusCodeInternalError = 1011,
    SRStatusCodeServiceRestart = 1012,
    SRStatusCodeTryAgainLater = 1013,
    // 1014: Reserved for future use by the WebSocket standard.
    SRStatusCodeTLSHandshake = 1015,
    // 1016–1999: Reserved for future use by the WebSocket standard.
    // 2000–2999: Reserved for use by WebSocket extensions.
    // 3000–3999: Available for use by libraries and frameworks. May not be used by applications. Available for registration at the IANA via first-come, first-serve.
    // 4000–4999: Available for use by applications.
} SRStatusCode;

@class SRWebSocket;

/**
 Error domain used for errors reported by SRWebSocket.
 */
extern NSString *const SRWebSocketErrorDomain;

/**
 Key used for HTTP status code if bad response was received from the server.
 */
extern NSString *const SRHTTPResponseErrorKey;

@protocol SRWebSocketDelegate;

#pragma mark - SRWebSocket

@interface SRWebSocket : NSObject <NSStreamDelegate>

/**
 The delegate of the web socket.

 The web socket delegate is notified on all state changes that happen to the web socket.
 */
@property (nonatomic, weak) id <SRWebSocketDelegate> delegate;

/**
 A dispatch queue for scheduling the delegate calls. The queue doesn't need be a serial queue.

 If `nil` and `delegateOperationQueue` is `nil`, the socket uses main queue for performing all delegate method calls.
 */
@property (nonatomic, strong) dispatch_queue_t delegateDispatchQueue;

/**
 An operation queue for scheduling the delegate calls.

 If `nil` and `delegateOperationQueue` is `nil`, the socket uses main queue for performing all delegate method calls.
 */
@property (nonatomic, strong) NSOperationQueue *delegateOperationQueue;

@property (nonatomic, readonly) SRReadyState readyState;
@property (nonatomic, readonly, retain) NSURL *url;

@property (nonatomic, readonly) CFHTTPMessageRef receivedHTTPHeaders;

// Optional array of cookies (NSHTTPCookie objects) to apply to the connections
@property (nonatomic, copy) NSArray<NSHTTPCookie *> *requestCookies;

// This returns the negotiated protocol.
// It will be nil until after the handshake completes.
@property (nonatomic, readonly, copy) NSString *protocol;

///--------------------------------------
#pragma mark - Constructors
///--------------------------------------

/**
 Initializes a web socket with a given `NSURLRequest`.

 @param request Request to initialize with.
 */
- (instancetype)initWithURLRequest:(NSURLRequest *)request;

/**
 Initializes a web socket with a given `NSURLRequest` and list of sub-protocols.

 @param request   Request to initialize with.
 @param protocols An array of strings that turn into `Sec-WebSocket-Protocol`. Default: `nil`.
 */
- (instancetype)initWithURLRequest:(NSURLRequest *)request protocols:(NSArray<NSString *> *)protocols;

/**
 Initializes a web socket with a given `NSURLRequest`, list of sub-protocols and whether untrusted SSL certificates are allowed.

 @param request                        Request to initialize with.
 @param protocols                      An array of strings that turn into `Sec-WebSocket-Protocol`.
 @param allowsUntrustedSSLCertificates Boolean value indicating whether untrusted SSL certificates are allowed. Default: `false`.
 */
- (instancetype)initWithURLRequest:(NSURLRequest *)request protocols:(NSArray<NSString *> *)protocols allowsUntrustedSSLCertificates:(BOOL)allowsUntrustedSSLCertificates;

/**
 Initializes a web socket with a given `NSURL`.

 @param url URL to initialize with.
 */
- (instancetype)initWithURL:(NSURL *)url;

/**
 Initializes a web socket with a given `NSURL` and list of sub-protocols.

 @param url       URL to initialize with.
 @param protocols An array of strings that turn into `Sec-WebSocket-Protocol`. Default: `nil`.
 */
- (instancetype)initWithURL:(NSURL *)url protocols:(NSArray<NSString *> *)protocols;

/**
 Initializes a web socket with a given `NSURL`, list of sub-protocols and whether untrusted SSL certificates are allowed.

 @param url                            URL to initialize with.
 @param protocols                      An array of strings that turn into `Sec-WebSocket-Protocol`.
 @param allowsUntrustedSSLCertificates Boolean value indicating whether untrusted SSL certificates are allowed. Default: `false`.
 */
- (instancetype)initWithURL:(NSURL *)url protocols:(NSArray<NSString *> *)protocols allowsUntrustedSSLCertificates:(BOOL)allowsUntrustedSSLCertificates;

///--------------------------------------
#pragma mark - Schedule
///--------------------------------------

/**
 Schedules a received on a given run loop in a given mode.
 By default, a web socket will schedule itself on `+[NSRunLoop SR_networkRunLoop]` using `NSDefaultRunLoopMode`.

 @param runLoop The run loop on which to schedule the receiver.
 @param mode     The mode for the run loop.
 */
- (void)scheduleInRunLoop:(NSRunLoop *)runLoop forMode:(NSString *)mode;

/**
 Removes the receiver from a given run loop running in a given mode.

 @param runLoop The run loop on which the receiver was scheduled.
 @param mode    The mode for the run loop.
 */
- (void)unscheduleFromRunLoop:(NSRunLoop *)runLoop forMode:(NSString *)mode;

///--------------------------------------
#pragma mark - Open / Close
///--------------------------------------

/**
 Opens web socket, which will trigger connection, authentication and start receiving/sending events.
 An instance of `SRWebSocket` is intended for one-time-use only. This method should be called once and only once.
 */
- (void)open;

/**
 Closes a web socket using `SRStatusCodeNormal` code and no reason.
 */
- (void)close;

/**
 Closes a web socket using a given code and reason.

 @param code   Code to close the socket with.
 @param reason Reason to send to the server or `nil`.
 */
- (void)closeWithCode:(NSInteger)code reason:(NSString *)reason;

///--------------------------------------
#pragma mark Send
///--------------------------------------

/**
 Send a UTF-8 string or binary data to the server.

 @param message UTF-8 String or Data to send.

 @deprecated Please use `sendString:` or `sendData` instead.
 */
- (void)send:(id)message __attribute__((deprecated("Please use `sendString:` or `sendData` instead.")));

/**
 Send a UTF-8 String to the server.

 @param string String to send.
 */
- (void)sendString:(NSString *)string;

/**
 Send binary data to the server.

 @param data Data to send.
 */
- (void)sendData:(NSData *)data;

/**
 Send Ping message to the server with optional data.

 @param data Instance of `NSData` or `nil`.
 */
- (void)sendPing:(NSData *)data;

@end

///--------------------------------------
#pragma mark - SRWebSocketDelegate
///--------------------------------------

@protocol SRWebSocketDelegate <NSObject>

@optional

#pragma mark Receive Messages

/**
 Called when any message was received from a web socket.
 This method is suboptimal and might be deprecated in a future release.

 @param webSocket An instance of `SRWebSocket` that received a message.
 @param message   Received message. Either a `String` or `NSData`.
 */
- (void)webSocket:(SRWebSocket *)webSocket didReceiveMessage:(id)message;

/**
 Called when a frame was received from a web socket.

 @param webSocket An instance of `SRWebSocket` that received a message.
 @param string    Received text in a form of UTF-8 `String`.
 */
- (void)webSocket:(SRWebSocket *)webSocket didReceiveMessageWithString:(NSString *)string;

/**
 Called when a frame was received from a web socket.

 @param webSocket An instance of `SRWebSocket` that received a message.
 @param data      Received data in a form of `NSData`.
 */
- (void)webSocket:(SRWebSocket *)webSocket didReceiveMessageWithData:(NSData *)data;

#pragma mark Status & Connection

- (void)webSocketDidOpen:(SRWebSocket *)webSocket;
- (void)webSocket:(SRWebSocket *)webSocket didFailWithError:(NSError *)error;
- (void)webSocket:(SRWebSocket *)webSocket didCloseWithCode:(NSInteger)code reason:(NSString *)reason wasClean:(BOOL)wasClean;
- (void)webSocket:(SRWebSocket *)webSocket didReceivePong:(NSData *)pongPayload;

// Return YES to convert messages sent as Text to an NSString. Return NO to skip NSData -> NSString conversion for Text messages. Defaults to YES.
- (BOOL)webSocketShouldConvertTextFrameToString:(SRWebSocket *)webSocket;

@end

#pragma mark - NSURLRequest (SRCertificateAdditions)

@interface NSURLRequest (SRCertificateAdditions)

@property (nonatomic, retain, readonly) NSArray *SR_SSLPinnedCertificates;

@end

#pragma mark - NSMutableURLRequest (SRCertificateAdditions)

@interface NSMutableURLRequest (SRCertificateAdditions)

@property (nonatomic, retain) NSArray *SR_SSLPinnedCertificates;

@end

#pragma mark - NSRunLoop (SRWebSocket)

@interface NSRunLoop (SRWebSocket)

+ (NSRunLoop *)SR_networkRunLoop;

@end
