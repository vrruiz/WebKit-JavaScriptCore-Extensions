#include <gtk/gtk.h>
#include <webkit/webkit.h>
#include <libnotify/notify.h>
#include <JavaScriptCore/JavaScript.h>

/* Class initialize */
static void notification_init_cb(JSContextRef ctx,
                                 JSObjectRef object)
{
    /* Inits notify */
    notify_init("Notification");
}

/* Notification.notify method callback implementation */
static JSValueRef notification_notify_cb(JSContextRef context,
                       JSObjectRef function,
                       JSObjectRef thisObject,
                       size_t argumentCount,
                       const JSValueRef arguments[],
                       JSValueRef *exception)
{
    /* At least, one argument must be received */
    if (argumentCount == 1 && JSValueIsString(context, arguments[0])) {
        /* Converts JSValue to char */
        size_t len;
        char *cstr;
        JSStringRef jsstr = JSValueToStringCopy(context, arguments[0], NULL);
        len = JSStringGetMaximumUTF8CStringSize(jsstr);
        cstr = g_new(char, len);
        JSStringGetUTF8CString(jsstr, cstr, len);

        /* Creates a new NotifyNotification. */
        NotifyNotification *notification = notify_notification_new(cstr, NULL, NULL);
        
        /* Sets the timeout of the notification. */
        notify_notification_set_timeout(notification, 3000);
        
        /* Sets the urgency level of this notification. */
        notify_notification_set_urgency(notification, NOTIFY_URGENCY_NORMAL);
        
        /* Tells the notification server to display the notification on the screen. */
        GError *error = NULL;
        notify_notification_show(notification, &error);
        
        g_object_unref(G_OBJECT(notification));
        g_free(cstr);
        
        JSStringRelease(jsstr);
    }
    
    return JSValueMakeUndefined(context);
}

/* Class method declarations */
static const JSStaticFunction notification_staticfuncs[] =
{
    { "notify", notification_notify_cb, kJSPropertyAttributeReadOnly },
    { NULL, NULL, 0 }
};

static const JSClassDefinition notification_def =
{
    0,                     // version
    kJSClassAttributeNone, // attributes
    "Notification",        // className
    NULL,                  // parentClass
    NULL,                  // staticValues
    notification_staticfuncs, // staticFunctions
    notification_init_cb,  // initialize
    NULL,                  // finalize
    NULL,                  // hasProperty
    NULL,                  // getProperty
    NULL,                  // setProperty
    NULL,                  // deleteProperty
    NULL,                  // getPropertyNames
    NULL,                  // callAsFunction
    NULL,                  // callAsConstructor
    NULL,                  // hasInstance  
    NULL                   // convertToType
};

/* Callback - JavaScript window object has been cleared */
static void window_object_cleared_cb(WebKitWebView  *web_view,
                                     WebKitWebFrame *frame,
                                     gpointer        context,
                                     gpointer        window_object,
                                     gpointer        user_data)

{
    /* Add classes to JavaScriptCore */
    JSClassRef classDef = JSClassCreate(&notification_def);
    JSObjectRef classObj = JSObjectMake(context, classDef, context);
    JSObjectRef globalObj = JSContextGetGlobalObject(context);
    JSStringRef str = JSStringCreateWithUTF8CString("Notification");
    JSObjectSetProperty(context, globalObj, str, classObj, kJSPropertyAttributeNone, NULL);
}

/* Destroy callback */
static void destroy(GtkWidget *widget,
                    gpointer   data )
{
    gtk_main_quit();
}

int
main (int argc, char* argv[])
{
    /* Initialize the widget set */
    gtk_init (&argc, &argv);
    
    /* Create the window widgets */
    GtkWidget *main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    GtkWidget *scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    
    /* Create the WebKit Web View widget */
    GtkWidget *web_view = webkit_web_view_new ();
    
    /* Connect the window object cleared event with callback */
    g_signal_connect (G_OBJECT (web_view), "window-object-cleared", G_CALLBACK(window_object_cleared_cb), web_view);


    /* Place the WebKitWebView in the GtkScrolledWindow */
    gtk_container_add (GTK_CONTAINER (scrolled_window), web_view);
    gtk_container_add (GTK_CONTAINER (main_window), scrolled_window);

    /* Connect the destroy window event with destroy function */
    g_signal_connect (G_OBJECT (main_window), "destroy", G_CALLBACK (destroy), NULL);
    
    /* Open webpage */
    webkit_web_view_load_uri (WEBKIT_WEB_VIEW (web_view), "file://webkit-03.html");

    /* Create the main window */
    gtk_window_set_default_size (GTK_WINDOW (main_window), 800, 600);
    
    /* Show the application window */
    gtk_widget_show_all (main_window);

    /* Enter the main event loop, and wait for user interaction */
    gtk_main ();
    
    /* The user lost interest */
    return 0;
}
