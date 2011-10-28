#include <stdlib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <gtk/gtk.h>
#include <webkit/webkit.h>
#include <JavaScriptCore/JavaScript.h>

DBusGConnection *conn;
DBusGProxy *proxy;
DBusGProxy *properties_proxy;

typedef struct {
    JSContextRef context;
    JSObjectRef object;
} RefContextObject;

/* Callback for UPower.Device.Changed() */
static void battery_changed_cb(DBusGProxy *_proxy, RefContextObject *ref) 
{
    g_message("Battery changed");
    
    /* Get onChange property */
    JSStringRef string_onchange;
    string_onchange = JSStringCreateWithUTF8CString("onChange");
    JSValueRef func = JSObjectGetProperty(ref->context, ref->object, string_onchange, NULL);
    JSObjectRef function = JSValueToObject(ref->context, func, NULL);
    JSStringRelease(string_onchange);
    
    if (!JSObjectIsFunction(ref->context, function)) {
         g_message("JSObject is not function or is not set");
         return;
    }
        
    JSValueRef result = JSObjectCallAsFunction(ref->context, // The execution context to use
                                               function, // The JSObject to call as a function.
                                               ref->object, // The object to use as "this," or NULL to use the global object as "this."
                                               0, //  An integer count of the number of arguments in arguments.
                                               NULL, // A JSValue array of arguments to pass to the function. Pass NULL if argumentCount is 0.
                                               NULL); // A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
        
}

/* Class initialize */
static void battery_init_cb(JSContextRef ctx,
                            JSObjectRef object)
{
    GError *error = NULL;
    
    conn = dbus_g_bus_get (DBUS_BUS_SYSTEM, &error);

    if (conn == NULL)
    {
        g_printerr ("Failed to open connection to bus: %s\n", error->message);
        g_error_free (error);
        return;
    }

    /* Create a proxy object for "org.freedesktop.UPower" */
    proxy = dbus_g_proxy_new_for_name (conn,
                                       "org.freedesktop.UPower",
                                       "/org/freedesktop/UPower/devices/battery_BAT0",
                                       "org.freedesktop.UPower.Device");
    if (proxy == NULL)
    {
        g_printerr ("Failed to create proxy object\n");
        return;
    }
    
    /* Create a proxy object for "org.freedesktop.UPower.Properties" */
    properties_proxy = dbus_g_proxy_new_from_proxy (proxy,
                                                    "org.freedesktop.DBus.Properties",
                                                    dbus_g_proxy_get_path (proxy));
    if (properties_proxy == NULL)
    {
        g_printerr ("Failed to create proxy object\n");
        return;
    }

    /* Connect callback to signal org.freedestop.UPower.Device.Changed() */
    RefContextObject *ref = g_new(RefContextObject, 1);
    ref->context = ctx;
    ref->object = object;
    
    dbus_g_proxy_add_signal (proxy, "Changed", G_TYPE_INVALID);
    dbus_g_proxy_connect_signal(proxy,
                                "Changed",
                                G_CALLBACK(battery_changed_cb),
                                ref,
                                (GClosureNotify) g_free);
  
    error = NULL;
}

/* Class constructor */
static JSObjectRef battery_constructor_cb(JSContextRef context,
                                          JSObjectRef constructor,
                                          size_t argumentCount,
                                          const JSValueRef arguments[],
                                          JSValueRef *exception)
{
    return constructor;
}                                       

/* Class finalize */
static void battery_destroy_cb(JSObjectRef object)
{
    /* Ends Battery */
    if (proxy != NULL) g_object_unref (proxy);
    if (properties_proxy != NULL) g_object_unref (properties_proxy);
}

static gboolean proxy_property_value(char *property,
				                     GValue *get_value,
                                     GError **error)
{
    /* Call ListNames method, wait for reply */
    return dbus_g_proxy_call (properties_proxy, "Get", error,
                              G_TYPE_STRING, "/org/freedesktop/UPower/devices/battery_BAT0",
                              G_TYPE_STRING, property,
                              G_TYPE_INVALID,
                              G_TYPE_VALUE, get_value,
                              G_TYPE_INVALID);
}

static JSValueRef proxy_double_value(JSContextRef context,
                                     char *property,
                                     size_t argumentCount)
{
    GError *error = NULL;
    GValue get_value = {0, };
  
    if (argumentCount == 0) {
        /* Call method, wait for reply */
        if (!proxy_property_value(property, &get_value, &error))
        {
            g_printerr ("Error: %s\n", error->message);
            g_error_free (error);
            return JSValueMakeUndefined(context);
        }
        
        gdouble value = g_value_get_double(&get_value);
        g_value_unset(&get_value);
        return JSValueMakeNumber(context, value);
    }
    
    return JSValueMakeUndefined(context);
}

static JSValueRef proxy_uint64_value(JSContextRef context,
                                     char *property,
                                     size_t argumentCount)
{
    GError *error = NULL;
    GValue get_value = {0, };
  
    if (argumentCount == 0) {
        /* Call method, wait for reply */
        if (!proxy_property_value(property, &get_value, &error))
        {
            g_printerr ("Error: %s\n", error->message);
            g_error_free (error);
            return JSValueMakeUndefined(context);
        }
        
        guint64 value = g_value_get_uint64(&get_value);
        g_value_unset(&get_value);
        return JSValueMakeNumber(context, value);
    }
    
    return JSValueMakeUndefined(context);
}

static JSValueRef proxy_boolean_value(JSContextRef context,
                                     char *property,
                                     size_t argumentCount)
{
    GError *error = NULL;
    GValue get_value = {0, };
  
    if (argumentCount == 0) {
        /* Call method, wait for reply */
        if (!proxy_property_value(property, &get_value, &error))
        {
            g_printerr ("Error: %s\n", error->message);
            g_error_free (error);
            return JSValueMakeUndefined(context);
        }
        
        gboolean value = g_value_get_boolean(&get_value);
        g_value_unset(&get_value);
        return JSValueMakeBoolean(context, value);
    }
    
    return JSValueMakeUndefined(context);
}

/* Battery.capacity method callback implementation */
static JSValueRef battery_capacity_cb(JSContextRef context,
                                      JSObjectRef function,
                                      JSObjectRef thisObject,
                                      size_t argumentCount,
                                      const JSValueRef arguments[],
                                      JSValueRef *exception)
{
    return proxy_double_value(context, "Capacity", argumentCount);
}

/* Battery.percentage method callback implementation */
static JSValueRef battery_percentage_cb(JSContextRef context,
                                        JSObjectRef function,
                                        JSObjectRef thisObject,
                                        size_t argumentCount,
                                        const JSValueRef arguments[],
                                        JSValueRef *exception)
{
    return proxy_double_value(context, "Percentage", argumentCount);
}

/* Battery.voltage method callback implementation */
static JSValueRef battery_voltage_cb(JSContextRef context,
                                     JSObjectRef function,
                                     JSObjectRef thisObject,
                                     size_t argumentCount,
                                     const JSValueRef arguments[],
                                     JSValueRef *exception)
{
    return proxy_double_value(context, "Voltage", argumentCount);
}

/* Battery.updateTime method callback implementation */
static JSValueRef battery_update_time_cb(JSContextRef context,
                                         JSObjectRef function,
                                         JSObjectRef thisObject,
                                         size_t argumentCount,
                                         const JSValueRef arguments[],
                                         JSValueRef *exception)
{
    return proxy_uint64_value(context, "UpdateTime", argumentCount);
}

/* Battery.PowerSupply method callback implementation */
static JSValueRef battery_power_supply_cb(JSContextRef context,
                                          JSObjectRef function,
                                          JSObjectRef thisObject,
                                          size_t argumentCount,
                                          const JSValueRef arguments[],
                                          JSValueRef *exception)
{
    return proxy_boolean_value(context, "PowerSupply", argumentCount);
}

/* Class method declarations */
static const JSStaticFunction battery_staticfuncs[] =
{
    { "capacity", battery_capacity_cb, kJSPropertyAttributeReadOnly },
    { "percentage", battery_percentage_cb, kJSPropertyAttributeReadOnly },
    { "voltage", battery_voltage_cb, kJSPropertyAttributeReadOnly },
    { "updateTime", battery_update_time_cb, kJSPropertyAttributeReadOnly },
    { "powerSupply", battery_power_supply_cb, kJSPropertyAttributeReadOnly },
    { NULL, NULL, 0 }
};

static const JSClassDefinition notification_def =
{
    0,                     // version
    kJSClassAttributeNone, // attributes
    "Battery",             // className
    NULL,                  // parentClass
    NULL,                  // staticValues
    battery_staticfuncs,   // staticFunctions
    battery_init_cb,       // initialize
    battery_destroy_cb,    // finalize
    NULL,                  // hasProperty
    NULL,                  // getProperty
    NULL,                  // setProperty
    NULL,                  // deleteProperty
    NULL,                  // getPropertyNames
    NULL,                  // callAsFunction
    battery_constructor_cb, // callAsConstructor
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
    JSStringRef str = JSStringCreateWithUTF8CString("Battery");
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
    webkit_web_view_load_uri (WEBKIT_WEB_VIEW (web_view), "file:///home/ubuntu/archivos/prog/c/webkit/webkit-05.html");

    /* Create the main window */
    gtk_window_set_default_size (GTK_WINDOW (main_window), 800, 600);
    
    /* Show the application window */
    gtk_widget_show_all (main_window);

    /* Enter the main event loop, and wait for user interaction */
    gtk_main ();
    
    /* The user lost interest */
    return 0;
}
