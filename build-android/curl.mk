SRC_PATH := $(call my-dir)/../ext/libcurl

LOCAL_PATH := $(SRC_PATH)

include $(CLEAR_VARS)

LOCAL_CPP_FEATURES += exceptions
LOCAL_CPP_FEATURES += rtti

LOCAL_MODULE := curl

### curl ###########################

LOCAL_SRC_FILES += \
	lib/asyn-ares.c \
	lib/asyn-thread.c \
	lib/axtls.c \
	lib/base64.c \
	lib/connect.c \
	lib/content_encoding.c \
	lib/cookie.c \
	lib/curl_addrinfo.c \
	lib/curl_fnmatch.c \
	lib/curl_gethostname.c \
	lib/curl_gssapi.c \
	lib/curl_memrchr.c \
	lib/curl_ntlm.c \
	lib/curl_ntlm_core.c \
	lib/curl_ntlm_msgs.c \
	lib/curl_ntlm_wb.c \
	lib/curl_rand.c \
	lib/curl_rtmp.c \
	lib/curl_sspi.c \
	lib/curl_threads.c \
	lib/cyassl.c \
	lib/dict.c \
	lib/easy.c \
	lib/escape.c \
	lib/file.c \
	lib/fileinfo.c \
	lib/formdata.c \
	lib/ftp.c \
	lib/ftplistparser.c \
	lib/getenv.c \
	lib/getinfo.c \
	lib/gopher.c \
	lib/gtls.c \
	lib/hash.c \
	lib/hmac.c \
	lib/hostasyn.c \
	lib/hostip.c \
	lib/hostip4.c \
	lib/hostip6.c \
	lib/hostsyn.c \
	lib/http.c \
	lib/http_chunks.c \
	lib/http_digest.c \
	lib/http_negotiate.c \
	lib/http_negotiate_sspi.c \
	lib/http_proxy.c \
	lib/if2ip.c \
	lib/imap.c \
	lib/inet_ntop.c \
	lib/inet_pton.c \
	lib/krb4.c \
	lib/krb5.c \
	lib/ldap.c \
	lib/llist.c \
	lib/md4.c \
	lib/md5.c \
	lib/memdebug.c \
	lib/mprintf.c \
	lib/multi.c \
	lib/netrc.c \
	lib/non-ascii.c \
	lib/nonblock.c \
	lib/nss.c \
	lib/nwlib.c \
	lib/nwos.c \
	lib/openldap.c \
	lib/parsedate.c \
	lib/pingpong.c \
	lib/polarssl.c \
	lib/pop3.c \
	lib/progress.c \
	lib/qssl.c \
	lib/rawstr.c \
	lib/rtsp.c \
	lib/security.c \
	lib/select.c \
	lib/sendf.c \
	lib/share.c \
	lib/slist.c \
	lib/smtp.c \
	lib/socks.c \
	lib/socks_gssapi.c \
	lib/socks_sspi.c \
	lib/speedcheck.c \
	lib/splay.c \
	lib/ssh.c \
	lib/sslgen.c \
	lib/ssluse.c \
	lib/strdup.c \
	lib/strequal.c \
	lib/strerror.c \
	lib/strtok.c \
	lib/strtoofft.c \
	lib/telnet.c \
	lib/tftp.c \
	lib/timeval.c \
	lib/transfer.c \
	lib/url.c \
	lib/version.c \
	lib/warnless.c \
	lib/wildcard.c \

### compile options

LOCAL_CFLAGS := \
	-DBUILDING_LIBCURL \
	-DHAVE_CONFIG_H \

LOCAL_ARM_MODE := arm

LOCAL_C_INCLUDES += \
	$(SRC_PATH)/include \

### export options

include $(BUILD_STATIC_LIBRARY)
