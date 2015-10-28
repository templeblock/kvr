/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

/*
* Copyright (c) 2015 Ubaka Onyechi
*
* kvr is free software distributed under the MIT license.
* See https://raw.githubusercontent.com/uonyx/kvr/master/LICENSE for details.
*/

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

#include "../src/kvr.h"
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

void sample_simple ()
{
  // create a kvr context
  kvr::ctx *ctx = kvr::ctx::create ();

  // create a map
  kvr::value *map = ctx->create_value ()->conv_map ();

  // populate map
  map->insert ("street", "sesame");
  map->insert ("flag", true);
  map->insert ("int", 123);
  map->insert ("pi", 3.1416);
  map->insert_null ("null");
  kvr::value *array = map->insert_array ("array");

  // populate child array
  array->push (256);
  array->push (42);
  array->push (-1);
  array->push (-255);

  // remove null value
  map->remove ("null");

  // iterate through the map and print keys
  kvr::pair p;
  kvr::value::cursor cur (map);
  while (cur.get (&p))
  {
    kvr::key *key = p.get_key ();
    printf ("%s\n", key->get_string ());
  }

  // change street value  
  map->find ("street")->set_string ("pigeon");

  // read value of 2nd element of map's array  
  int64_t i = map->find ("array")->element (1)->get_integer ();
  if (i == 42)
    printf ("%s\n", "got it");

  // clean up
  ctx->destroy_value (map);

  // destroy context
  kvr::ctx::destroy (ctx);
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

int main ()
{
  sample_simple ();

  return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
