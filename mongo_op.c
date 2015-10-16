#include <bson.h>
#include <mongoc.h>
#include <stdio.h>
#include "protocal2929.h"

mongo_save_gps(ontent_gps_up *g )
{
        //保存mongodb
    bson_oid_t oid;
    bson_t *doc;

    unsigned char timegps[15];
    unsigned char latitude[10];
    unsigned char longitude[10];
    double lat;
    double lng;
    doc = bson_new ();

    bson_oid_init (&oid, NULL);
    BSON_APPEND_OID (doc, "_id", &oid);
    BSON_APPEND_INT32 (doc, "id",get_product_id(g->terminal_id));

    get_gps_time(g->base_info.date_time, timegps );
    
    BSON_APPEND_UTF8 (doc, "timestamp", timegps);
    
    lat = bcd2longitude(g->base_info.latitude, latitude );
    lng = bcd2longitude(g->base_info.longitude, longitude );
   
    BSON_APPEND_UTF8 (doc, "latitude", latitude);
    BSON_APPEND_UTF8 (doc, "longitude", longitude);
    BSON_APPEND_DOUBLE (doc, "lat", lat);
    BSON_APPEND_DOUBLE (doc, "lng", lng);
    BSON_APPEND_INT32 (doc, "speed", htons(g->base_info.speed));
    BSON_APPEND_INT32 (doc, "direction", htons(g->base_info.direction));
    BSON_APPEND_INT32 (doc, "high", htons(g->base_info.high));
    BSON_APPEND_INT32 (doc, "pos_station", g->base_info.pos_station);

    if (!mongoc_collection_insert (collection, MONGOC_INSERT_NONE, doc, NULL, &error)) {
        printf ("Insert failed: %s\n", error.message);
    }

    bson_destroy (doc);
}


mongo_get_command(ontent_gps_up *g )
{
        //保存mongodb
    bson_oid_t oid;
    bson_t *doc;

    unsigned char timegps[15];
    unsigned char latitude[10];
    unsigned char longitude[10];
    double lat;
    double lng;
    doc = bson_new ();

    query = bson_new ();
    BSON_APPEND_UTF8 (query, "hello", "world");

    cursor = mongoc_collection_find (collection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);

    while (mongoc_cursor_next (cursor, &doc)) {
        str = bson_as_json (doc, NULL);
        printf ("%s\n", str);
        bson_free (str);
    }

    bson_destroy (query);


    bson_destroy (doc);
}