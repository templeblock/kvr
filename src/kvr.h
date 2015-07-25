/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef KVR_H
#define KVR_H

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

#if defined (_MSC_VER)
#define KVR_DEBUG          _DEBUG
#else
#define KVR_DEBUG          DEBUG
#endif

#if __cplusplus >= 201103L
#define KVR_CPP11 1
#else
#define KVR_CPP11 0
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

#if KVR_CPP11
#include <unordered_map>
#define std_unordered_map std::unordered_map
#else
#if defined (_MSC_VER)
#include <unordered_map>
#else
#include <tr1/unordered_map>
#endif
#define std_unordered_map std::tr1::unordered_map
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

#define KVR_OPTIMIZATION_IMPLICIT_TYPE_CONVERSION_OFF   0   // type conversion is on you
#define KVR_OPTIMIZATION_FAST_MAP_INSERT_ON             0   // ignore duplicate keys in maps
#define KVR_OPTIMIZATION_FAST_MAP_REMOVE_ON             0   // no ownership check - be careful

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

// ideal to have the first 3 constants constants statically bound by client (aka templates)

#define KVR_CONSTANT_ZERO_TOLERANCE                     (1.0e-7)
#define KVR_CONSTANT_MAX_KEY_LENGTH                     (127)
#define KVR_CONSTANT_MAX_TREE_DEPTH                     (64)
#define KVR_CONSTANT_TOKEN_MAP_GREP                      '@'
#define KVR_CONSTANT_TOKEN_DELIMITER                     '.'

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

class kvr
{
public:
  
  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////

  typedef uint16_t sz_t;

#if KVR_DEBUG
  static const uint64_t MAX_SZ_T = (1 << (sizeof (sz_t) * 8)) - 1;
#endif

  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////

  enum data_format
  {
    DATA_FORMAT_JSON,
    DATA_FORMAT_MSGPACK,
  };

  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////

  class strbuffer;
  class pair;

  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////

  class key
  {
  public:

    const char *  get_string () const;
    sz_t          get_length () const;

  private:

    key (const char *str);
    key (char *str, sz_t len);
    ~key ();

    char *  m_str;
    sz_t    m_len;
    sz_t    m_ref;

    friend class kvr;
  };

  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////
  
  class value
  {
  public:

    ///////////////////////////////////////////
    ///////////////////////////////////////////
    ///////////////////////////////////////////

    class cursor;    

    ///////////////////////////////////////////
    ///////////////////////////////////////////
    ///////////////////////////////////////////

    // type checking
    bool          is_null () const;
    bool          is_map () const;
    bool          is_array () const;
    bool          is_string () const;
    bool          is_boolean () const;
    bool          is_number_i () const;
    bool          is_number_f () const;
    
    // type conversion
    value *       conv_null ();
    value *       conv_map ();
    value *       conv_array ();
    value *       conv_string ();
    value *       conv_boolean ();
    value *       conv_number_i ();
    value *       conv_number_f ();

    // native variant ops
    void          set_string (const char *str);
    void          set_boolean (bool b);
    void          set_number_i (int64_t n);
    void          set_number_f (double n);
    const char *  get_string () const;
    bool          get_boolean () const;
    int64_t       get_number_i () const;
    double        get_number_f () const;

    // array variant ops
    value *       push (int64_t number);
    value *       push (double number);
    value *       push (bool boolean);
    value *       push (const char *str);
    value *       push_map ();
    value *       push_array ();
    value *       push_null ();
    bool          pop ();
    value *       element (sz_t index) const;
    sz_t          size () const;

    // map variant ops
    pair *        find (const char *key) const;
    pair *        insert (const char *key, int64_t number);
    pair *        insert (const char *key, double number);
    pair *        insert (const char *key, bool boolean);
    pair *        insert (const char *key, const char *str);
    pair *        insert_map (const char *key);
    pair *        insert_array (const char *key);
    pair *        insert_null (const char *key);    
    bool          remove (pair *node);
    cursor        fcursor () const;

    // copy
    value *       copy (const value *rhs);

    // path search (map or array)
    value *       search (const char *pathexpr) const;
    value *       search (const char **path, sz_t pathsz) const;

    // serialize/deserialize
    bool          serialize (data_format format, strbuffer *strbuf) const;
    bool          serialize (data_format format, std::string *str) const;
    bool          deserialize (data_format format, const char *str);

    // diff/patch
    bool          diff (const value *original, const value *modified); 
    bool          patch (const value *diff);
    
    // hash code
    uint32_t      hashcode (uint32_t seed = 0) const;

    // debug output
    void          dump () const;

  private:

    ///////////////////////////////////////////
    ///////////////////////////////////////////
    ///////////////////////////////////////////

    union string
    {
      struct dyn_str
      {
        static const sz_t PAD = 8;

        char *  data;
        sz_t    size;
        sz_t    len;
      } m_dyn;

      struct stt_str
      {
        static const sz_t CAP = ((sizeof (sz_t) * 2) + sizeof (char *));

        char data [CAP];
      } m_stt;
    };

    ///////////////////////////////////////////
    ///////////////////////////////////////////
    ///////////////////////////////////////////

    union number
    {
      int64_t   i;
      double    f;
    };

    ///////////////////////////////////////////
    ///////////////////////////////////////////
    ///////////////////////////////////////////

    struct array
    {
      static const sz_t CAP_INCR = 8;

      void    init (sz_t size);
      void    deinit ();
      void    push (value *v);
      value * pop ();
      value * pop (sz_t index);
      value * elem (sz_t index) const;

      value **m_ptr;
      sz_t    m_len;
      sz_t    m_cap;
    };

    ///////////////////////////////////////////
    ///////////////////////////////////////////
    ///////////////////////////////////////////

    struct map
    {
      static const sz_t CAP_INCR = 8;

      void    init (sz_t size);
      void    deinit ();
      pair *  insert (key *k, value *v);
      bool    remove (pair *p);
      pair *  find (const key *k) const;

      pair *  m_ptr;
      sz_t    m_size;
      sz_t    m_cap;     
    };

  public:

    ///////////////////////////////////////////
    ///////////////////////////////////////////
    ///////////////////////////////////////////

    class cursor
    {
    public:

      pair *  get ();

    private:

      cursor (const map *m) : m_map (m), m_index (0) {}      
#if KVR_CPP11
      cursor (cursor &&c) : m_map (c.m_map), m_index (c.m_index) {}
#endif
      const map * m_map;
      sz_t        m_index;

      friend class value;
    };

  private:

    ///////////////////////////////////////////
    ///////////////////////////////////////////
    ///////////////////////////////////////////

    union data
    {
      number    n;
      map       m;
      array     a;
      string    s;
      bool      b;
    };

    ///////////////////////////////////////////
    ///////////////////////////////////////////
    ///////////////////////////////////////////

    bool    _is_number () const;
    bool    _is_string_dynamic () const;
    bool    _is_string_static () const;

    value * _conv_map (sz_t size = map::CAP_INCR);
    value * _conv_array (sz_t size = array::CAP_INCR);

    sz_t    _get_string_length () const;
    sz_t    _get_string_size () const;
    void    _set_string (const char *str, sz_t len);
    void    _set_string_stt (const char *str);
    void    _set_string_dyn (const char *str, sz_t size);
    void    _move_string (char *str, sz_t size);

    value * _search_path_expr (const char *expr, const char **lastkey = NULL, 
                               value **lastparent = NULL) const;
    value * _search_key (const char *key) const;
    
    uint8_t _type () const;
    void    _destruct ();
    void    _clear ();    
    void    _dump (size_t lpad, const char *key) const;

    void    _diff_set (value *set, value *rem, const value *og, const value *md,
                       const char **path, const sz_t pathsz, sz_t pathcnt);
    void    _diff_add (value *add, const value *og, const value *md,
                       const char **path, const sz_t pathsz, sz_t pathcnt);

    void    _patch_set (const value *set);
    void    _patch_add (const value *add);
    void    _patch_rem (const value *rem);
   
    pair  * _insert_kv (key *k, value *v);
    void    _push_v (value *v);

    ///////////////////////////////////////////
    ///////////////////////////////////////////
    ///////////////////////////////////////////

    value (kvr *ctx, uint32_t flags);
    ~value ();

    ///////////////////////////////////////////
    ///////////////////////////////////////////
    ///////////////////////////////////////////

    data      m_data; 
    uint32_t  m_flags;
    kvr     * m_ctx;
    
    friend class kvr;
    friend class kvr_internal;
  };

  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////

  class pair
  {
  public:

    key   *       get_key () const;
    value *       get_value ();

  private:

    pair () : m_k (NULL), m_v (NULL) {}

    key   * m_k;
    value * m_v;

    friend class kvr;
  };
  
  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////

  class strbuffer
  {
  public:

    const char *  get_string () const;
    size_t        get_length () const;

    strbuffer ();
    ~strbuffer ();

  private:

    strbuffer (const strbuffer &);
    strbuffer &operator= (const strbuffer &);

    void _reset ();

    char  * m_str;
    size_t  m_len;

    friend class kvr;
  };

  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////

  static kvr *  create_context (uint32_t flags = 0);
  static void   destroy_context (kvr *ctx);
  
  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////
  
  value * create_value ();
  void    destroy_value (value *v);
   
  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////

private:

  kvr ();
  kvr (const kvr &);
  ~kvr ();

  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////

  enum value_flags
  {
    VALUE_FLAG_NONE                 = 0,
    VALUE_FLAG_TYPE_NULL            = (1 << 0),
    VALUE_FLAG_TYPE_MAP             = (1 << 1),
    VALUE_FLAG_TYPE_ARRAY           = (1 << 2),
    VALUE_FLAG_TYPE_DYN_STRING      = (1 << 3),
    VALUE_FLAG_TYPE_STT_STRING      = (1 << 4),
    VALUE_FLAG_TYPE_NUMBER_INTEGER  = (1 << 5),
    VALUE_FLAG_TYPE_NUMBER_FLOAT    = (1 << 6),
    VALUE_FLAG_TYPE_BOOLEAN         = (1 << 7),
    VALUE_FLAG_PARENT_CTX           = (1 << 8),
    VALUE_FLAG_PARENT_MAP           = (1 << 9),
    VALUE_FLAG_PARENT_ARRAY         = (1 << 10),
  };

  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////

  struct equal_cstr
  {
    bool operator ()(const char *a, const char *b) const
    {
      return strcmp (a, b) == 0;
    }
  };

  struct hash_djb
  {
    uint32_t operator ()(const char *s) const
    {
      uint32_t hash = 5381;
      char c;

      while ((c = *s++))
      {
        hash = ((hash << 5) + hash) + c;
      }

      return hash;
    }
  };

  typedef std_unordered_map<const char *, key *, hash_djb, equal_cstr> keystore;
  
  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////

  value * _create_value_null (uint32_t parentType);
  value * _create_value_map (uint32_t parentType);
  value * _create_value_array (uint32_t parentType);
  value * _create_value (uint32_t parentType, int64_t number);
  value * _create_value (uint32_t parentType, double number);
  value * _create_value (uint32_t parentType, bool boolean);
  value * _create_value (uint32_t parentType, const char *str, sz_t len);
  void    _destroy_value (uint32_t parentType, value *v);

  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////

  key *   _find_key (const char *str);  
  key *   _create_key_copy (const char *str);
  key *   _create_key_move (char *str, sz_t len);
  void    _destroy_key (key *k);

  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////

  char *  _create_path_expr (const char **path, sz_t pathsz, sz_t *exprsz = NULL) const;
  void    _destroy_path_expr (char *expr);

  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////

  keystore m_keystore;
};

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

inline const char *kvr::key::get_string () const
{
  return m_str;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

inline kvr::sz_t kvr::key::get_length () const
{
  return m_len;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

inline bool kvr::value::is_null () const
{
  return (m_flags & VALUE_FLAG_TYPE_NULL) != 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

inline bool kvr::value::is_map () const
{
  return (m_flags & VALUE_FLAG_TYPE_MAP) != 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

inline bool kvr::value::is_array () const
{
  return (m_flags & VALUE_FLAG_TYPE_ARRAY) != 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

inline bool kvr::value::is_string () const
{
  return (m_flags & (VALUE_FLAG_TYPE_DYN_STRING | VALUE_FLAG_TYPE_STT_STRING)) != 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

inline bool kvr::value::is_boolean () const
{
  return (m_flags & VALUE_FLAG_TYPE_BOOLEAN) != 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

inline bool kvr::value::_is_number () const
{
  return (m_flags & (VALUE_FLAG_TYPE_NUMBER_INTEGER | VALUE_FLAG_TYPE_NUMBER_FLOAT)) != 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

inline bool kvr::value::is_number_i () const
{
  return (m_flags & VALUE_FLAG_TYPE_NUMBER_INTEGER) != 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

inline bool kvr::value::is_number_f () const
{
  return (m_flags & VALUE_FLAG_TYPE_NUMBER_FLOAT) != 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

inline bool kvr::value::_is_string_dynamic () const
{
  return (m_flags & VALUE_FLAG_TYPE_DYN_STRING) != 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

inline bool kvr::value::_is_string_static () const
{
  return (m_flags & VALUE_FLAG_TYPE_STT_STRING) != 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

inline kvr::key * kvr::pair::get_key () const
{
  return m_k;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

inline kvr::value * kvr::pair::get_value ()
{
  return m_v;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

#endif

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
