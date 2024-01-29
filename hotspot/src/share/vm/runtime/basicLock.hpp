/*
 * Copyright (c) 1998, 2010, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#ifndef SHARE_VM_RUNTIME_BASICLOCK_HPP
#define SHARE_VM_RUNTIME_BASICLOCK_HPP

#include "oops/markOop.hpp"
#include "runtime/handles.hpp"
#include "utilities/top.hpp"

class BasicLock VALUE_OBJ_CLASS_SPEC {
  friend class VMStructs;
 private:
  volatile markOop _displaced_header;  // markOopDesc对象指针， 即8b
 public:
  markOop      displaced_header() const               { return _displaced_header; }
  void         set_displaced_header(markOop header)   { _displaced_header = header; }

  void print_on(outputStream* st) const;

  // move a basic lock (used during deoptimization
  void move_to(oop obj, BasicLock* dest);

  static int displaced_header_offset_in_bytes()       { return offset_of(BasicLock, _displaced_header); }
};

// A BasicObjectLock associates a specific Java object with a BasicLock.
// It is currently embedded in an interpreter frame.

// Because some machines have alignment restrictions on the control stack,
// the actual space allocated by the interpreter may include padding words
// after the end of the BasicObjectLock.  Also, in order to guarantee
// alignment of the embedded BasicLock objects on such machines, we
// put the embedded BasicLock at the beginning of the struct.

// 一个 BasicObjectLock 将1个Java 对象与一个 BasicLock 相关联。它当前嵌入在一个解释器帧（interpreter frame）中。
// 总结：1个BasicObjectLock会放入解释器栈中，代表对应的java对象与1个BasicLock对象
// 
// 由于某些机器对控制栈(control stack)有对齐(alignment)限制，解释器（interpreter）实际分配的空间可能包含BasicObjectLock末尾填充字（padding words）。
// 此外，为了保证在这些机器上的'内嵌BasicLock对象'的对齐，我们将'内嵌BasicLock对象' 放置在结构体的开头。
// 总结：保证在栈上也对齐，BasicObjectLock后也会使用在末尾padding来对齐，且把BasicLock放在开头
class BasicObjectLock VALUE_OBJ_CLASS_SPEC {
  friend class VMStructs;
 private:
  BasicLock _lock;                                    // the lock, must be double word aligned
                                                      // 具体lock对象，必须2word 对齐（1word64位即8b），即64位上是16b对齐（没满则填充） 
                                                      // BasicLock里占用8b 
  oop       _obj;                                     // object holds the lock; 使用这个锁的java对象
  // 

 public:
  // Manipulation
  oop      obj() const                                { return _obj;  }
  void set_obj(oop obj)                               { _obj = obj; }
  BasicLock* lock()                                   { return &_lock; }

  // Note: Use frame::interpreter_frame_monitor_size() for the size of BasicObjectLocks
  //       in interpreter activation frames since it includes machine-specific padding.
  static int size()                                   { return sizeof(BasicObjectLock)/wordSize; }

  // GC support
  void oops_do(OopClosure* f) { f->do_oop(&_obj); }

  static int obj_offset_in_bytes()                    { return offset_of(BasicObjectLock, _obj);  }
  static int lock_offset_in_bytes()                   { return offset_of(BasicObjectLock, _lock); }
};


#endif // SHARE_VM_RUNTIME_BASICLOCK_HPP
