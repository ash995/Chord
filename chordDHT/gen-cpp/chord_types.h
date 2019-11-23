/**
 * Autogenerated by Thrift Compiler (0.14.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef chord_TYPES_H
#define chord_TYPES_H

#include <iosfwd>

#include <thrift/Thrift.h>
#include <thrift/TApplicationException.h>
#include <thrift/TBase.h>
#include <thrift/protocol/TProtocol.h>
#include <thrift/transport/TTransport.h>

#include <functional>
#include <memory>




class NODE;

class NODE_INFO;

typedef struct _NODE__isset {
  _NODE__isset() : id(false), ip(false) {}
  bool id :1;
  bool ip :1;
} _NODE__isset;

class NODE : public virtual ::apache::thrift::TBase {
 public:

  NODE(const NODE&);
  NODE& operator=(const NODE&);
  NODE() : id(0), ip() {
  }

  virtual ~NODE() noexcept;
  int64_t id;
  std::string ip;

  _NODE__isset __isset;

  void __set_id(const int64_t val);

  void __set_ip(const std::string& val);

  bool operator == (const NODE & rhs) const
  {
    if (!(id == rhs.id))
      return false;
    if (!(ip == rhs.ip))
      return false;
    return true;
  }
  bool operator != (const NODE &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const NODE & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  virtual void printTo(std::ostream& out) const;
};

void swap(NODE &a, NODE &b);

std::ostream& operator<<(std::ostream& out, const NODE& obj);

typedef struct _NODE_INFO__isset {
  _NODE_INFO__isset() : me(false), succ(false), pred(false), f_table(false) {}
  bool me :1;
  bool succ :1;
  bool pred :1;
  bool f_table :1;
} _NODE_INFO__isset;

class NODE_INFO : public virtual ::apache::thrift::TBase {
 public:

  NODE_INFO(const NODE_INFO&);
  NODE_INFO& operator=(const NODE_INFO&);
  NODE_INFO() {
  }

  virtual ~NODE_INFO() noexcept;
  NODE me;
  NODE succ;
  NODE pred;
  std::vector<NODE>  f_table;

  _NODE_INFO__isset __isset;

  void __set_me(const NODE& val);

  void __set_succ(const NODE& val);

  void __set_pred(const NODE& val);

  void __set_f_table(const std::vector<NODE> & val);

  bool operator == (const NODE_INFO & rhs) const
  {
    if (!(me == rhs.me))
      return false;
    if (!(succ == rhs.succ))
      return false;
    if (!(pred == rhs.pred))
      return false;
    if (!(f_table == rhs.f_table))
      return false;
    return true;
  }
  bool operator != (const NODE_INFO &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const NODE_INFO & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  virtual void printTo(std::ostream& out) const;
};

void swap(NODE_INFO &a, NODE_INFO &b);

std::ostream& operator<<(std::ostream& out, const NODE_INFO& obj);



#endif
