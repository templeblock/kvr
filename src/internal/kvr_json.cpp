/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

#include "kvr_json.h"
#include "kvr_internal.h"
#include "rapidjson/reader.h"
#include "rapidjson/writer.h"
#include "rapidjson/error/en.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
// json_read_context
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

struct json_read_context
{
  json_read_context (kvr::value *value) : m_root (value), m_pair (NULL), m_depth (0)
  {
    memset (m_stack, 0, sizeof (m_stack));
  }

  ///////////////////////////////////////////
  ///////////////////////////////////////////
  ///////////////////////////////////////////

  bool Null ()
  {
    bool success = true;

    KVR_ASSERT_SAFE (m_depth != 0, success);
    kvr::value *node = m_stack [m_depth - 1];
    KVR_ASSERT (node);
    KVR_ASSERT (node->is_map () || node->is_array ());

    if (node->is_map ())
    {
      KVR_ASSERT_SAFE (m_pair, false);
      kvr::value *pv = m_pair->get_value ();
      KVR_ASSERT_SAFE (pv && pv->is_null (), false);
      m_pair = NULL;
    }
    else if (node->is_array ())
    {
      node->push_null ();
    }
    else
    {
      success = false;
    }

    return success;
  }

  bool Bool (bool b)
  {
    bool success = true;

    KVR_ASSERT_SAFE (m_depth != 0, success);
    kvr::value *node = m_stack [m_depth - 1];
    KVR_ASSERT (node);
    KVR_ASSERT (node->is_map () || node->is_array ());

    if (node->is_map ())
    {
      KVR_ASSERT_SAFE (m_pair, false);
      kvr::value *pv = m_pair->get_value ();
      KVR_ASSERT_SAFE (pv && pv->is_null (), false);
#if KVR_OPTIMIZATION_IMPLICIT_TYPE_CONVERSION_OFF
      pv->conv_boolean ();
#endif
      pv->set_boolean (b);
      m_pair = NULL;
    }
    else if (node->is_array ())
    {
      node->push (b);
    }
    else
    {
      success = false;
    }

    return success;
  }

  bool Int (int i)
  {
    return Int64 ((int64_t) i);
  }

  bool Uint (unsigned u)
  {
    return Int64 ((int64_t) u);
  }

  bool Int64 (int64_t i)
  {
    bool success = true;

    KVR_ASSERT_SAFE (m_depth != 0, success);
    kvr::value *node = m_stack [m_depth - 1];
    KVR_ASSERT (node);
    KVR_ASSERT (node->is_map () || node->is_array ());

    if (node->is_map ())
    {
      KVR_ASSERT_SAFE (m_pair, false);
      kvr::value *pv = m_pair->get_value ();
      KVR_ASSERT_SAFE (pv && pv->is_null (), false);
#if KVR_OPTIMIZATION_IMPLICIT_TYPE_CONVERSION_OFF
      pv->conv_integer ();
#endif
      pv->set_integer (i);
      m_pair = NULL;
    }
    else if (node->is_array ())
    {
      node->push (i);
    }
    else
    {
      success = false;
    }

    return success;
  }

  bool Uint64 (uint64_t u)
  {
    KVR_ASSERT (m_depth != 0);
    KVR_ASSERT (false && "not supported");
    return false;
  }

  bool Double (double d)
  {
    bool success = true;

    KVR_ASSERT_SAFE (m_depth != 0, success);
    kvr::value *node = m_stack [m_depth - 1];
    KVR_ASSERT (node);
    KVR_ASSERT (node->is_map () || node->is_array ());

    if (node->is_map ())
    {
      KVR_ASSERT_SAFE (m_pair, false);
      kvr::value *pv = m_pair->get_value ();
      KVR_ASSERT_SAFE (pv && pv->is_null (), false);
#if KVR_OPTIMIZATION_IMPLICIT_TYPE_CONVERSION_OFF
      pv->conv_float ();
#endif
      pv->set_float (d);
      m_pair = NULL;
    }
    else if (node->is_array ())
    {
      node->push (d);
    }
    else
    {
      success = false;
    }

    return success;
  }

  bool String (const char *str, kvr_rapidjson::SizeType length, bool copy)
  {
    bool success = true;

    KVR_ASSERT_SAFE (m_depth != 0, success);
    kvr::value *node = m_stack [m_depth - 1];
    KVR_ASSERT (node);
    KVR_ASSERT (node->is_map () || node->is_array ());

    if (node->is_map ())
    {
      KVR_ASSERT_SAFE (m_pair, false);
      kvr::value *pv = m_pair->get_value ();
      KVR_ASSERT_SAFE (pv && pv->is_null (), false);
      pv->conv_string ();
      pv->set_string (str, (kvr::sz_t) length);
      m_pair = NULL;
    }
    else if (node->is_array ())
    {
      kvr::value *vstr = node->push_null (); KVR_ASSERT (vstr);
      vstr->conv_string ();
      vstr->set_string (str, (kvr::sz_t) length);
    }
    else
    {
      success = false;
    }

    return success;
  }

  bool StartObject ()
  {
    bool success = true;
    kvr::value *node = NULL;

    if (m_depth > 0)
    {
      node = m_stack [m_depth - 1];
      KVR_ASSERT (node);
      KVR_ASSERT (node->is_map () || node->is_array ());

      if (node->is_map ())
      {
        KVR_ASSERT_SAFE (m_pair, false);
        node = m_pair->get_value ();
        KVR_ASSERT_SAFE (node && node->is_null (), false);
        node->conv_map ();
        m_pair = NULL;
      }
      else if (node->is_array ())
      {
        node = node->push_map ();
      }
      else
      {
        success = false;
      }
    }
    else
    {
      node = m_root->conv_map ();
    }

    KVR_ASSERT (m_depth < KVR_CONSTANT_MAX_TREE_DEPTH);
    m_stack [m_depth++] = node;

    return success;
  }

  bool Key (const char *str, kvr_rapidjson::SizeType length, bool copy)
  {
    kvr::value *node = m_stack [m_depth - 1];
    KVR_ASSERT_SAFE (node && node->is_map (), false);

    KVR_ASSERT (!m_pair);
    m_pair = node->insert_null (str);
    return (m_pair != NULL);
  }

  bool EndObject (kvr_rapidjson::SizeType memberCount)
  {
    kvr::value *node = m_stack [m_depth - 1];
    KVR_ASSERT_SAFE (node && node->is_map (), false);
    KVR_ASSERT (node->size () == (kvr::sz_t) memberCount);
    m_stack [--m_depth] = NULL;
    return true;
  }

  bool StartArray ()
  {
    bool success = true;
    kvr::value *node = NULL;

    if (m_depth > 0)
    {
      node = m_stack [m_depth - 1];
      KVR_ASSERT (node);
      KVR_ASSERT (node->is_map () || node->is_array ());

      if (node->is_map ())
      {
        KVR_ASSERT_SAFE (m_pair, false);
        node = m_pair->get_value ();
        KVR_ASSERT_SAFE (node && node->is_null (), false);
        node->conv_array ();
        m_pair = NULL;
      }
      else if (node->is_array ())
      {
        node = node->push_array ();
      }
      else
      {
        success = false;
      }
    }
    else
    {
      node = m_root->conv_array ();
    }

    KVR_ASSERT (m_depth < KVR_CONSTANT_MAX_TREE_DEPTH);
    m_stack [m_depth++] = node;

    return success;
  }

  bool EndArray (kvr_rapidjson::SizeType elementCount)
  {
    kvr::value *node = m_stack [m_depth - 1];
    KVR_ASSERT_SAFE (node && node->is_array (), false);
    KVR_ASSERT (node->length () == (kvr::sz_t) elementCount);
    m_stack [--m_depth] = NULL;
    return true;
  }

  ///////////////////////////////////////////
  ///////////////////////////////////////////
  ///////////////////////////////////////////

  kvr::value  * m_stack [KVR_CONSTANT_MAX_TREE_DEPTH];
  kvr::value  * m_root;
  kvr::pair   * m_pair;
  kvr::sz_t     m_depth;
};

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
// json_write_context
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

struct json_write_context
{
  json_write_context (kvr::ostream *ostream) : m_stream (ostream)
  {
    KVR_ASSERT (ostream);

    m_stream->seek (0);
    m_stream->set_eos (0);
  }

  ///////////////////////////////////////////
  ///////////////////////////////////////////
  ///////////////////////////////////////////

  void Put (char ch)
  {
    bool ok = m_stream->put (ch);
    if (!ok)
    {
      size_t newcap = m_stream->capacity () * 2;
      m_stream->resize (newcap);
      ok = m_stream->put (ch);
    }
    KVR_ASSERT (ok);
  }

  char *Push (size_t count)
  {
    uint8_t *bytes = m_stream->push (count);
    if (!bytes)
    {
      size_t newcap = m_stream->capacity () * 2;
      m_stream->resize (newcap);
      bytes = m_stream->push (count);
    }
    KVR_ASSERT (bytes);
    return (char *) bytes;
  }

  char *Pop (size_t count)
  {
    uint8_t *bytes = m_stream->pop (count);
    KVR_ASSERT (bytes);
    return (char *) bytes;
  }

  void Flush () {}

  ///////////////////////////////////////////
  ///////////////////////////////////////////
  ///////////////////////////////////////////
  
  kvr::ostream      *m_stream;

  ///////////////////////////////////////////
  ///////////////////////////////////////////
  ///////////////////////////////////////////

  static bool write_stream (const kvr::value *val, kvr_rapidjson::Writer<json_write_context> &writer);
  static size_t write_approx_size (const kvr::value *val);
};

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

bool json_write_context::write_stream (const kvr::value *val, kvr_rapidjson::Writer<json_write_context> &writer)
{
  bool success = false;

  //////////////////////////////////
  if (val->is_map ())
  //////////////////////////////////
  {
    bool ok = writer.StartObject ();
    kvr::value::cursor c = val->fcursor ();
    kvr::pair *p = c.get ();
    while (p && ok)
    {
      kvr::key *k = p->get_key ();
      writer.Key (k->get_string (), k->get_length ());

      kvr::value *v = p->get_value ();
      ok = write_stream (v, writer);

      p = c.get ();
    }
    success = ok && writer.EndObject ();
  }

  //////////////////////////////////
  else if (val->is_array ())
  //////////////////////////////////
  {
    bool ok = writer.StartArray ();
    for (kvr::sz_t i = 0, c = val->length (); (i < c) && ok; ++i)
    {
      kvr::value *v = val->element (i);
      ok = write_stream (v, writer);
    }
    success = ok && writer.EndArray ();
  }

  //////////////////////////////////
  else if (val->is_string ())
  //////////////////////////////////
  {
    kvr::sz_t slen = 0;
    const char *str = val->get_string (&slen);
    success = writer.String (str, slen);
  }

  //////////////////////////////////
  else if (val->is_integer ())
  //////////////////////////////////
  {
    int64_t n = val->get_integer ();
    success = writer.Int64 (n);
  }

  //////////////////////////////////
  else if (val->is_float ())
  //////////////////////////////////
  {
    double n = val->get_float ();
    success = writer.Double (n);
  }

  //////////////////////////////////
  else if (val->is_boolean ())
  //////////////////////////////////
  {
    bool b = val->get_boolean ();
    success = writer.Bool (b);
  }

  //////////////////////////////////
  else if (val->is_null ())
  //////////////////////////////////
  {
    success = writer.Null ();
  }

  return success;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

size_t json_write_context::write_approx_size (const kvr::value *val)
{
  size_t size = 0;

  //////////////////////////////////
  if (val->is_map ())
  //////////////////////////////////
  {
    size += 2; // brackets
    kvr::value::cursor c = val->fcursor ();
    kvr::pair  *p = c.get ();
    while (p)
    {
      kvr::key *k = p->get_key ();
      kvr::value *v = p->get_value ();

      size += k->get_length () + 2; // + quotes
      size += write_approx_size (v);
      size += 2; // colon and comma

      p = c.get ();
    }
  }

  //////////////////////////////////
  else if (val->is_array ())
  //////////////////////////////////
  {
    size += 2; // brackets
    for (kvr::sz_t i = 0, c = val->length (); i < c; ++i)
    {
      kvr::value *v = val->element (i);
      size += kvr_internal::ndigitsu32 (i);
      size += write_approx_size (v);
      size += 1; // comma
    }
  }

  //////////////////////////////////
  else if (val->is_string ())
  //////////////////////////////////
  {
    kvr::sz_t slen = 0;
    const char *str = val->get_string (&slen);
    size += (slen + 2); // + quotes
  }

  //////////////////////////////////
  else if (val->is_integer ())
  //////////////////////////////////
  {
    int64_t n = val->get_integer ();
    size += kvr_internal::ndigitsi64 (n);
  }

  //////////////////////////////////
  else if (val->is_float ())
  //////////////////////////////////
  {
    double n = val->get_float ();

    if (n < std::numeric_limits<float>::max ())
    {
      size += 13; // average (guess) 
    }
    else
    {
      size += 25; // 25 is max
    }    
  }

  //////////////////////////////////
  else if (val->is_boolean ())
  //////////////////////////////////
  {
    bool b = val->get_boolean ();
    size += b ? 4 : 5;
  }

  //////////////////////////////////
  else if (val->is_null ())
  //////////////////////////////////
  {
    size += 4;
  }

  return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
// kvr_json
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

bool kvr_json::read (kvr::value *dest, const char *str, size_t len)
{
  KVR_ASSERT (dest);
  KVR_ASSERT (str);
  KVR_ASSERT (len > 0);

  bool success = false;

  json_read_context rctx (dest);
  kvr_rapidjson::StringStream ss (str);
  kvr_rapidjson::Reader reader;

  kvr_rapidjson::ParseResult ok = reader.Parse (ss, rctx);
#if KVR_DEBUG
  if (ok.IsError ()) { fprintf (stderr, "JSON parse error: %s (%lu)", kvr_rapidjson::GetParseError_En (ok.Code ()), ok.Offset ()); }
#endif
  success = ok && (rctx.m_depth == 0);

  return success;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

bool kvr_json::write (const kvr::value *src, kvr::ostream *ostr)
{
  KVR_ASSERT (ostr);

  bool success = false;

  json_write_context wctx (ostr);
  kvr_rapidjson::Writer<json_write_context> writer (wctx);
  success = json_write_context::write_stream (src, writer);
  ostr->set_eos (0);

  return success;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

size_t kvr_json::write_approx_size (const kvr::value *val)
{
  return json_write_context::write_approx_size (val);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
