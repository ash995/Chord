/**
 * Autogenerated by Thrift Compiler (0.14.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#include "chord_types.h"

#include <algorithm>
#include <ostream>

#include <thrift/TToString.h>




NODE::~NODE() noexcept {
}


void NODE::__set_id(const int64_t val) {
  this->id = val;
}

void NODE::__set_ip(const std::string& val) {
  this->ip = val;
}
std::ostream& operator<<(std::ostream& out, const NODE& obj)
{
  obj.printTo(out);
  return out;
}


uint32_t NODE::read(::apache::thrift::protocol::TProtocol* iprot) {

  ::apache::thrift::protocol::TInputRecursionTracker tracker(*iprot);
  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;


  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
        if (ftype == ::apache::thrift::protocol::T_I64) {
          xfer += iprot->readI64(this->id);
          this->__isset.id = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 2:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->ip);
          this->__isset.ip = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  return xfer;
}

uint32_t NODE::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  ::apache::thrift::protocol::TOutputRecursionTracker tracker(*oprot);
  xfer += oprot->writeStructBegin("NODE");

  xfer += oprot->writeFieldBegin("id", ::apache::thrift::protocol::T_I64, 1);
  xfer += oprot->writeI64(this->id);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("ip", ::apache::thrift::protocol::T_STRING, 2);
  xfer += oprot->writeString(this->ip);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

void swap(NODE &a, NODE &b) {
  using ::std::swap;
  swap(a.id, b.id);
  swap(a.ip, b.ip);
  swap(a.__isset, b.__isset);
}

NODE::NODE(const NODE& other0) {
  id = other0.id;
  ip = other0.ip;
  __isset = other0.__isset;
}
NODE& NODE::operator=(const NODE& other1) {
  id = other1.id;
  ip = other1.ip;
  __isset = other1.__isset;
  return *this;
}
void NODE::printTo(std::ostream& out) const {
  using ::apache::thrift::to_string;
  out << "NODE(";
  out << "id=" << to_string(id);
  out << ", " << "ip=" << to_string(ip);
  out << ")";
}


NODE_INFO::~NODE_INFO() noexcept {
}


void NODE_INFO::__set_me(const NODE& val) {
  this->me = val;
}

void NODE_INFO::__set_succ(const NODE& val) {
  this->succ = val;
}

void NODE_INFO::__set_pred(const NODE& val) {
  this->pred = val;
}

void NODE_INFO::__set_f_table(const std::vector<NODE> & val) {
  this->f_table = val;
}
std::ostream& operator<<(std::ostream& out, const NODE_INFO& obj)
{
  obj.printTo(out);
  return out;
}


uint32_t NODE_INFO::read(::apache::thrift::protocol::TProtocol* iprot) {

  ::apache::thrift::protocol::TInputRecursionTracker tracker(*iprot);
  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;


  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
        if (ftype == ::apache::thrift::protocol::T_STRUCT) {
          xfer += this->me.read(iprot);
          this->__isset.me = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 2:
        if (ftype == ::apache::thrift::protocol::T_STRUCT) {
          xfer += this->succ.read(iprot);
          this->__isset.succ = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 3:
        if (ftype == ::apache::thrift::protocol::T_STRUCT) {
          xfer += this->pred.read(iprot);
          this->__isset.pred = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 4:
        if (ftype == ::apache::thrift::protocol::T_LIST) {
          {
            this->f_table.clear();
            uint32_t _size2;
            ::apache::thrift::protocol::TType _etype5;
            xfer += iprot->readListBegin(_etype5, _size2);
            this->f_table.resize(_size2);
            uint32_t _i6;
            for (_i6 = 0; _i6 < _size2; ++_i6)
            {
              xfer += this->f_table[_i6].read(iprot);
            }
            xfer += iprot->readListEnd();
          }
          this->__isset.f_table = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  return xfer;
}

uint32_t NODE_INFO::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  ::apache::thrift::protocol::TOutputRecursionTracker tracker(*oprot);
  xfer += oprot->writeStructBegin("NODE_INFO");

  xfer += oprot->writeFieldBegin("me", ::apache::thrift::protocol::T_STRUCT, 1);
  xfer += this->me.write(oprot);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("succ", ::apache::thrift::protocol::T_STRUCT, 2);
  xfer += this->succ.write(oprot);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("pred", ::apache::thrift::protocol::T_STRUCT, 3);
  xfer += this->pred.write(oprot);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("f_table", ::apache::thrift::protocol::T_LIST, 4);
  {
    xfer += oprot->writeListBegin(::apache::thrift::protocol::T_STRUCT, static_cast<uint32_t>(this->f_table.size()));
    std::vector<NODE> ::const_iterator _iter7;
    for (_iter7 = this->f_table.begin(); _iter7 != this->f_table.end(); ++_iter7)
    {
      xfer += (*_iter7).write(oprot);
    }
    xfer += oprot->writeListEnd();
  }
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

void swap(NODE_INFO &a, NODE_INFO &b) {
  using ::std::swap;
  swap(a.me, b.me);
  swap(a.succ, b.succ);
  swap(a.pred, b.pred);
  swap(a.f_table, b.f_table);
  swap(a.__isset, b.__isset);
}

NODE_INFO::NODE_INFO(const NODE_INFO& other8) {
  me = other8.me;
  succ = other8.succ;
  pred = other8.pred;
  f_table = other8.f_table;
  __isset = other8.__isset;
}
NODE_INFO& NODE_INFO::operator=(const NODE_INFO& other9) {
  me = other9.me;
  succ = other9.succ;
  pred = other9.pred;
  f_table = other9.f_table;
  __isset = other9.__isset;
  return *this;
}
void NODE_INFO::printTo(std::ostream& out) const {
  using ::apache::thrift::to_string;
  out << "NODE_INFO(";
  out << "me=" << to_string(me);
  out << ", " << "succ=" << to_string(succ);
  out << ", " << "pred=" << to_string(pred);
  out << ", " << "f_table=" << to_string(f_table);
  out << ")";
}


