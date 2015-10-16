#include <bcon.h>
#include <bson.h>
#include <mongoc.h>
#include <stdio.h>
#define MAX_STANDBY 6
int
main (int   argc,
      char *argv[])
{
    mongoc_collection_t *collection_state;
    mongoc_client_t *client;
    bson_error_t error;
    bson_oid_t oid;
    bson_t *doc = NULL;
    bson_t *update = NULL;
    bson_t *query = NULL;
    int count;
    int found = 0;

    mongoc_cursor_t *cursor;



    mongoc_init ();


    client = mongoc_client_new ("mongodb://localhost:27017/");
    collection_state = mongoc_client_get_collection (client, "test", "test");

    bson_oid_init (&oid, NULL);
    query = bson_new ();
    BSON_APPEND_INT32 (query, "id",12);

    found = mongoc_collection_count (collection_state, MONGOC_QUERY_NONE, query, 0, 0, NULL, &error);

    if (found == 0) {
        doc = bson_new ();
        bson_oid_init (&oid, NULL);
        BSON_APPEND_INT32 (doc, "id", 12);
        BSON_APPEND_INT32 (doc, "count", 0);

        if (!mongoc_collection_insert (collection_state, MONGOC_INSERT_NONE, doc, NULL, &error)) {
            printf ("%s\n", error.message);
        }

    }

    query = bson_new ();
    BSON_APPEND_INT32 (query, "id", 12);

    cursor = mongoc_collection_find (collection_state, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);

    //while (mongoc_cursor_next (cursor, &doc)) {
    //   mongoc_cursor_next (cursor, &result);
    while (mongoc_cursor_next (cursor, (const  bson_t **) &doc)) {
        bson_iter_t iter;
        bson_iter_t sub_iter;
        // str = bson_as_json (doc, NULL);
        // fprintf (stderr, "%s\n", str);
        // bson_free (str);

        if (bson_iter_init (&iter, doc) && bson_iter_find_descendant (&iter, "count", &sub_iter)) {
            // fprintf (stderr,"Found key \"%s\" in sub document.\n", bson_iter_key (&sub_iter));
            // printf ("The type of a.b.c.d is: %d\n", (int)bson_iter_type (&sub_iter));
            count = (int)bson_iter_int32 (&sub_iter);
        }
    }
    fprintf (stderr,"Found count %d .\n", count);

    count++;
    
   if (count > MAX_STANDBY) 
    {
        count = 1; 
        update = bson_new ();
        BSON_APPEND_INT32 (update, "id",12);
        BSON_APPEND_INT32 (update, "count", count);


        if (!mongoc_collection_update (collection_state, MONGOC_UPDATE_NONE, query, update, NULL, &error)) {
            printf ("%s\n", error.message);
        }
    }
    else
    {
        //send 38

        update = bson_new ();
        BSON_APPEND_INT32 (update, "id",12);
        BSON_APPEND_INT32 (update, "count", count);

        if (!mongoc_collection_update (collection_state, MONGOC_UPDATE_NONE, query, update, NULL, &error)) {
            printf ("%s\n", error.message);
        }
    }

fail:
    if (doc)
        bson_destroy (doc);
    if (query)
        bson_destroy (query);
    if (update)
        bson_destroy (update);

    mongoc_collection_destroy (collection_state);
    mongoc_client_destroy (client);

    return 0;
}