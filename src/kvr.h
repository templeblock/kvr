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
#define KVR_INLINE         __forceinline
#else
#define KVR_DEBUG          DEBUG
#define KVR_INLINE         inline
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

#if _WIN32 || _WIN64
#if _WIN64
#define KVR_64
#else
#define KVR_32
#endif
#endif

#if __GNUC__ || __clang__
#if __x86_64__ || __ppc64__
#define KVR_64
#else
#define KVR_32
#endif
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

// ideal to have the first 3 constants constants statically bound by client (aka templates)

#define KVR_CONSTANT_ZERO_TOLERANCE                     (1.0e-7)
#define KVR_CONSTANT_MAX_KEY_LENGTH                     (127ULL)
#define KVR_CONSTANT_MAX_TREE_DEPTH                     (64)
#define KVR_CONSTANT_TOKEN_MAP_GREP                      '@'
#define KVR_CONSTANT_TOKEN_DELIMITER                     '.'
#define KVR_CONSTANT_COMMON_BLOCK_SZ                    (8)

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

class kvr
{
public:
  
  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////

#if defined (KVR_64)
  typedef uint32_t sz_t;
#if KVR_DEBUG
  static const uint64_t SZ_T_MAX = 0xffffffff; 
#endif
#else
  typedef uint16_t sz_t;
#if KVR_DEBUG
  static const uint64_t SZ_T_MAX = 0xffff;
#endif
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

  class buffer;
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
    bool          is_map () const;
    bool          is_array () const;
    bool          is_string () const;
    bool          is_boolean () const;
    bool          is_integer () const;
    bool          is_float () const;
    bool          is_null () const;
    
    // type conversion    
    value *       conv_map (sz_t cap = KVR_CONSTANT_COMMON_BLOCK_SZ);
    value *       conv_array (sz_t cap = KVR_CONSTANT_COMMON_BLOCK_SZ);
    value *       conv_string ();
    value *       conv_boolean ();
    value *       conv_integer ();
    value *       conv_float ();
    value *       conv_null ();

    // native variant ops
    void          set_string (const char *str, sz_t len);
    void          set_string (const char *str);
    void          set_boolean (bool b);
    void          set_integer (int64_t n);
    void          set_float (double n);

    const char *  get_string () const;
    const char *  get_string (sz_t *len) const;
    bool          get_boolean () const;
    int64_t       get_integer () const;
    double        get_float () const;

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
    sz_t          length () const;

    // map variant ops
    value *       insert (const char *key, int64_t number);
    value *       insert (const char *key, double number);
    value *       insert (const char *key, bool boolean);
    value *       insert (const char *key, const char *str);
    value *       insert_map (const char *key);
    value *       insert_array (const char *key);
    value *       insert_null (const char *key);
    value *       find (const char *key) const;
    bool          remove (const char *key);
    cursor        fcursor () const;
    sz_t          size () const;

    // copy
    value *       copy (const value *rhs);

    // path search (map or array)
    value *       search (const char *pathexpr) const;
    value *       search (const char **path, sz_t pathsz) const;

    // serialize/deserialize
    bool          serialize (data_format format, buffer *buf) const;
    bool          deserialize (data_format format, const uint8_t *data, size_t sz);        
    size_t        approx_serialize_size (data_format format) const;

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
        static const sz_t PAD = (KVR_CONSTANT_COMMON_BLOCK_SZ - 1);

        char *  data;
        sz_t    size;
        sz_t    len;
      } m_dyn;

      struct stt_str
      {
        static const sz_t CAP = ((sizeof (sz_t) * 2) + sizeof (char *)); // sizeof dyn_str

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
      static const sz_t CAP_INCR = KVR_CONSTANT_COMMON_BLOCK_SZ;

      void    init (sz_t size);
      void    deinit ();
      void    push (value *v);
      value * pop ();
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
      static const sz_t CAP_INCR = KVR_CONSTANT_COMMON_BLOCK_SZ;

      struct node
      {
        key   *k;
        value *v;
      };

      void    init (sz_t size);
      void    deinit ();
      node *  insert (key *k, value *v);
      void    remove (node *n);
      node *  find (const key *k) const;
      sz_t    size_l () const;
      sz_t    size_c () const;
      sz_t    _cap () const; // true cap

      node *  m_ptr;
      sz_t    m_len;
      sz_t    m_cap;
    };

  public:

    ///////////////////////////////////////////
    ///////////////////////////////////////////
    ///////////////////////////////////////////

    class cursor
    {
    public:

      bool get (pair *pair);

    private:

      const map::node * _get ();

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
    
    sz_t    _get_string_length () const;
    sz_t    _get_string_size () const;
    void    _set_string (const char *str, sz_t len);
    void    _set_string_stt (const char *str, sz_t len);
    void    _set_string_dyn (const char *str, sz_t len);
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
   
    bool    _insert_kv (key *k, value *v);
    void    _push_v (value *v);

    value * _conv_map (sz_t cap);
    value * _conv_array (sz_t cap);

    sz_t    _size1 () const;
    sz_t    _size2 () const;

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
  };

  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////

  class pair
  {
  public:

    key   *       get_key () const;
    value *       get_value ();

    pair () : m_k (NULL), m_v (NULL) {}

  private:

    key   * m_k;
    value * m_v;

    friend class kvr;
  };
  
  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////

  class ostream
  {
  public:

    ostream (size_t cap);
    ~ostream ();
    
    const uint8_t * bytes () const;
    size_t          capacity () const;
    size_t          tell () const;    
    bool            full () const;
    void            seek (size_t pos);
    bool            put (uint8_t byte);
    bool            put (uint8_t *bytes, size_t count);
    uint8_t *       push (size_t count);
    uint8_t *       pop (size_t count);
    void            set_eos (uint8_t eos);
    void            resize (size_t newcap);

  private:
    
    uint8_t *       alloc (size_t size);
    void            free (uint8_t *bytes);

    ostream (const ostream &);
    ostream &operator=(const ostream &);

    uint8_t * m_bytes;
    size_t    m_cap;
    size_t    m_pos;
    bool      m_heap;
  };

  class istream
  {
  public:

    istream (const uint8_t *bytes, size_t size);
    
    const uint8_t * bytes () const;
    size_t          size () const;
    size_t          tell () const;
    bool            end () const;
    void            seek (size_t pos);
    bool            get (uint8_t *byte);
    bool            get (uint8_t *bytes, size_t count);
    const uint8_t * push (size_t count);
    const uint8_t * pop (size_t count);
    
  private:

    istream (const istream &);
    istream &operator=(const istream &);

    const uint8_t * m_bytes;
    size_t    m_size;
    size_t    m_pos;
  };

  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////

  class buffer
  {
  public:

    buffer (size_t capacity = DEFAULT_CAPACITY) : m_stream (capacity) {}

    const uint8_t * get_data () const;
    size_t          get_size () const;

  private:
    
    static const int DEFAULT_CAPACITY = 256;

    ostream m_stream;
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

KVR_INLINE const char *kvr::key::get_string () const
{
  return m_str;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

KVR_INLINE kvr::sz_t kvr::key::get_length () const
{
  return m_len;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

KVR_INLINE bool kvr::value::is_null () const
{
  return (m_flags & VALUE_FLAG_TYPE_NULL) != 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

KVR_INLINE bool kvr::value::is_map () const
{
  return (m_flags & VALUE_FLAG_TYPE_MAP) != 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

KVR_INLINE bool kvr::value::is_array () const
{
  return (m_flags & VALUE_FLAG_TYPE_ARRAY) != 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

KVR_INLINE bool kvr::value::is_string () const
{
  return (m_flags & (VALUE_FLAG_TYPE_DYN_STRING | VALUE_FLAG_TYPE_STT_STRING)) != 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

KVR_INLINE bool kvr::value::is_boolean () const
{
  return (m_flags & VALUE_FLAG_TYPE_BOOLEAN) != 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

KVR_INLINE bool kvr::value::is_integer () const
{
  return (m_flags & VALUE_FLAG_TYPE_NUMBER_INTEGER) != 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

KVR_INLINE bool kvr::value::is_float () const
{
  return (m_flags & VALUE_FLAG_TYPE_NUMBER_FLOAT) != 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

KVR_INLINE bool kvr::value::_is_string_dynamic () const
{
  return (m_flags & VALUE_FLAG_TYPE_DYN_STRING) != 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

KVR_INLINE bool kvr::value::_is_string_static () const
{
  return (m_flags & VALUE_FLAG_TYPE_STT_STRING) != 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

KVR_INLINE bool kvr::value::_is_number () const
{
  return (m_flags & (VALUE_FLAG_TYPE_NUMBER_INTEGER | VALUE_FLAG_TYPE_NUMBER_FLOAT)) != 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

KVR_INLINE kvr::key * kvr::pair::get_key () const
{
  return m_k;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

KVR_INLINE kvr::value * kvr::pair::get_value ()
{
  return m_v;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

KVR_INLINE const uint8_t * kvr::buffer::get_data () const
{
  return m_stream.bytes ();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

KVR_INLINE size_t kvr::buffer::get_size () const
{
  return m_stream.tell ();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

#endif

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
