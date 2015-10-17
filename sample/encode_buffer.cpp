//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "../src/kvr.h"
#include <stdio.h>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void sample_encode_buffer ()
{
  // create a kvr context
  kvr::ctx *ctx = kvr::ctx::create ();

  // create a map and populate
  kvr::value *map = ctx->create_value ()->conv_map ();
  map->insert ("street", "sesame");
  map->insert ("f", false);
  map->insert ("i", 123);
  map->insert ("pi", 3.1416);
  map->insert_null ("null");
  map->insert_array ("array")->push (-1);

  // create val (we'll use this for decoding)
  kvr::value *val = ctx->create_value ();

  // encode to msgpack using kvr::obuffer with default capacity
  {
    kvr::obuffer obuf;
    if (map->encode (kvr::CODEC_MSGPACK, &obuf))
    {
      // decode data into val
      if (val->decode (kvr::CODEC_MSGPACK, obuf.get_data (), obuf.get_size ()))
      {
        double pi = val->find ("pi")->get_float ();
        printf ("pi in val is %f\n", pi);
      }
    }
  }

  // encode to json using kvr::obuffer with approximated capacity
  {
    size_t bufsz = map->calculate_encode_size (kvr::CODEC_JSON);
    kvr::obuffer obuf (bufsz);
    if (map->encode (kvr::CODEC_JSON, &obuf))
    {
      // decode data into val
      if (val->decode (kvr::CODEC_JSON, obuf.get_data (), obuf.get_size ()))
      {
        double pi = val->find ("pi")->get_float ();
        printf ("pi in val is %f\n", pi);
      }
    }
  }

  // encode to cbor using kvr::obuffer with custom buffer
  {
    uint8_t data [1024];
    kvr::obuffer obuf (data, 1024);
    if (map->encode (kvr::CODEC_CBOR, &obuf))
    {
      // decode data into val
      if (val->decode (kvr::CODEC_CBOR, obuf.get_data (), obuf.get_size ()))
      {
        double pi = val->find ("pi")->get_float ();
        printf ("pi in val is %f\n", pi);
      }
    }
  }

  // clean up
  ctx->destroy_value (map);
  ctx->destroy_value (val);

  // destroy context
  kvr::ctx::destroy (ctx);
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int main ()
{
  sample_encode_buffer ();

  return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
