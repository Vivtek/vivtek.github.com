#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#ifdef WINDOWS
#include <winsock.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif
#include "../wftk.h"
#include "../wftk_internals.h"
static char *names[] = 
{
   "init",
   "free",
   "info",
   "send"
};

XML * NOTIFY_smtp_init (WFTK_ADAPTOR * ad, va_list args);
XML * NOTIFY_smtp_free (WFTK_ADAPTOR * ad, va_list args);
XML * NOTIFY_smtp_info (WFTK_ADAPTOR * ad, va_list args);
XML * NOTIFY_smtp_send (WFTK_ADAPTOR * ad, va_list args);

static WFTK_API_FUNC vtab[] = 
{
   NOTIFY_smtp_init,
   NOTIFY_smtp_free,
   NOTIFY_smtp_info,
   NOTIFY_smtp_send
};

static struct adaptor_info _NOTIFY_smtp_info =
{
   4,
   names,
   vtab
};
struct adaptor_info * NOTIFY_smtp_get_info ()
{
   return & _NOTIFY_smtp_info;
}
XML * NOTIFY_smtp_init (WFTK_ADAPTOR * ad, va_list args) {
   const char * parms;
   char spec[256];

   parms = xml_attrval (ad->parms, "parm");
   if (!*parms) parms = config_get_value (ad->session, "notify.stmp.server");
   if (!*parms) {
      xml_set (ad->parms, "error", "No SMTP server specified and no default exists.");
      return (XML *) 0;
   }

   xml_set (ad->parms, "server", parms);
   strcpy (spec, "smtp:");
   strcat (spec, xml_attrval (ad->parms, "server"));
   xml_set (ad->parms, "spec", spec);
   return (XML *) 0;
}
XML * NOTIFY_smtp_free (WFTK_ADAPTOR * ad, va_list args) { return (XML *) 0; }
XML * NOTIFY_smtp_info (WFTK_ADAPTOR * ad, va_list args) {
   XML * info;

   info = xml_create ("info");
   xml_set (info, "type", "notify");
   xml_set (info, "name", "smtp");
   xml_set (info, "ver", "1.0.0");
   xml_set (info, "compiled", __TIME__ " " __DATE__);
   xml_set (info, "author", "Michael Roberts");
   xml_set (info, "contact", "wftk@vivtek.com");
   xml_set (info, "extra_functions", "0");

   return (info);
}
XML * NOTIFY_smtp_send  (WFTK_ADAPTOR * ad, va_list args)
{
   XML * context = (XML *) 0;
   XML * alert = (XML *) 0;

   char hostname[64];
   struct hostent *server;
   char line[1024];
   char spec[1024];
   int bytes;
   char * text;
   char * mark;
   char * textptr;
#ifdef WINDOWS
   SOCKET sock;
#  define SOCK_BAD (sock == INVALID_SOCKET)
#  define CLOSESOCK (closesocket (sock))
   WSADATA wsa;
#else
   int sock;
#  define SOCK_BAD (sock < 0)
#  define CLOSESOCK (close (sock))
#endif
   struct sockaddr_in name;

   if (args) context = va_arg (args, XML *);
   if (!context) {
      xml_set (ad->parms, "error", "No context specified.");
      return (XML *) 0;
   }
   if (args) alert = va_arg (args, XML *);
   if (!alert) {
      xml_set (ad->parms, "error", "No message specified.");
      return (XML *) 0;
   }

#ifdef WINDOWS
   WSAStartup (MAKEWORD (1, 0), &wsa);
#endif

   gethostname (hostname, sizeof(hostname));

   server = gethostbyname (xml_attrval (ad->parms, "server"));
   if (!server) {
      xml_set (ad->parms, "error", "Unable to resolve server name.");
      return (XML *) 0;
   }

   sock = socket (AF_INET, SOCK_STREAM, 0);
   if (SOCK_BAD) {
      xml_set (ad->parms, "error", "Unable to allocate socket.");
      return (XML *) 0;
   }

   memset (&name, 0, sizeof (struct sockaddr_in));
   name.sin_family = AF_INET;
   name.sin_port = htons (25);
   memcpy (&name.sin_addr, server->h_addr_list[0], server->h_length);

   if (connect(sock, (struct sockaddr *) &name, sizeof (struct sockaddr)) < 0) {
      xml_set (ad->parms, "error", "Unable to connect to server.");
      CLOSESOCK;
      return (XML *) 0;
   }

   memset (line, 0, sizeof(line));
   bytes = recv (sock, line, sizeof(line), 0);

   sprintf (line, "HELO %s\r\n", hostname);
   send (sock, line, strlen(line), 0);  /* Send HELO. */
   memset (line, 0, sizeof(line));
   bytes = recv (sock, line, sizeof(line), 0);
   if (bytes < 4 || strncmp (line, "250 ", 4)) {
      xml_set (ad->parms, "error", "Server refused connection.");
      CLOSESOCK;
      return (XML *) 0;
   }

   sprintf (line, "MAIL FROM: %s\r\n", xml_attrval (alert, "from_addr"));
   send (sock, line, strlen(line), 0);
   memset (line, 0, sizeof(line));
   bytes = recv (sock, line, sizeof(line), 0);
   if (bytes < 4 || strncmp (line, "250 ", 4)) {
      xml_set (ad->parms, "error", "Failure on MAIL FROM:");
      CLOSESOCK;
      return (XML *) 0;
   }

   sprintf (line, "RCPT TO: %s\r\n", xml_attrval (alert, "to_addr"));
   send (sock, line, strlen(line), 0);
   memset (line, 0, sizeof(line));
   bytes = recv (sock, line, sizeof(line), 0);
   if (bytes < 4 || strncmp (line, "250 ", 4)) {
      xml_set (ad->parms, "error", "Failure on RCPT TO:");
      CLOSESOCK;
      return (XML *) 0;
   }

   send (sock, "DATA\r\n", 6, 0);
   bytes = recv (sock, line, sizeof(line), 0);

   sprintf (line, "From: %s <%s>\n", xml_attrval (alert, "from_name"), xml_attrval (alert, "from_addr"));
   send (sock, line, strlen(line), 0);
   sprintf (line, "To: %s <%s>\n", xml_attrval (alert, "to_name"), xml_attrval (alert, "to_addr"));
   send (sock, line, strlen(line), 0);
   sprintf (spec, "Subject: %s\n", xml_attrval (alert, "subject"));
   wftk_value_interpret (ad->session, context, spec, line, sizeof(line));
   send (sock, line, strlen(line), 0);
   send (sock, "\n", 1, 0);

   text = xml_stringcontent (alert);
   textptr = text;
   mark = strchr (textptr, '\n');
   while (mark) {
      strncpy (spec, textptr, mark - textptr);
      spec[mark-textptr] = '\0';
      if (*spec && (strspn (spec, ".") == strlen (spec))) strcat (spec, ".");
      strcat (spec, "\n");
      
      wftk_value_interpret (ad->session, context, spec, line, sizeof (line));

      send (sock, line, strlen(line), 0);

      textptr = mark + 1;
      mark = strchr (textptr, '\n');
   }

   strcpy (spec, textptr);
   if (*spec && (strspn (spec, ".") == strlen (spec))) strcat (spec, ".");
   strcat (spec, "\n");
   wftk_value_interpret (ad->session, context, spec, line, sizeof (line));
   send (sock, line, strlen(line), 0);

   free (text);

   send (sock, ".\r\n", 3, 0);
   memset (line, 0, sizeof(line));
   bytes = recv (sock, line, sizeof(line), 0);
   if (bytes < 4 || strncmp (line, "250 ", 4)) {
      xml_set (ad->parms, "error", "Message not accepted by server.");
      CLOSESOCK;
      return (XML *) 0;
   }

   send (sock, "QUIT\r\n", 6, 0);
   bytes = recv (sock, line, sizeof(line), 0);

   CLOSESOCK;
   return (XML *) 0;
}
