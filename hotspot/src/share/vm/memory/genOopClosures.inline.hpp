/*
 * Copyright (c) 2001, 2012, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_VM_MEMORY_GENOOPCLOSURES_INLINE_HPP
#define SHARE_VM_MEMORY_GENOOPCLOSURES_INLINE_HPP

#include "memory/cardTableRS.hpp"
#include "memory/defNewGeneration.hpp"
#include "memory/genCollectedHeap.hpp"
#include "memory/genOopClosures.hpp"
#include "memory/genRemSet.hpp"
#include "memory/generation.hpp"
#include "memory/sharedHeap.hpp"
#include "memory/space.hpp"

inline OopsInGenClosure::OopsInGenClosure(Generation* gen) :
  ExtendedOopClosure(gen->ref_processor()), _orig_gen(gen), _rs(NULL) {
  set_generation(gen);
}

// 就是设置当前代信息到闭包
inline void OopsInGenClosure::set_generation(Generation* gen) {
  _gen = gen;// 更新当前处理的代
   // 拿到对应代的用于card marking的的内存地址开始
  _gen_boundary = _gen->reserved().start();

  // Barrier set for the heap, must be set after heap is initialized（为堆准备的barrier set，一定在堆初始化后set）
  // 没有设置rs（卡表）则获取
  if (_rs == NULL) {
    // 理论上SharedHeap::heap()就是通过static拿回当前使用的heap对象（只会一个）
    // 然后拿回rs（_rem_set 记忆集）
    // 分代GenCollectedHeap：CardTableRS
    // g1 -》 G1CollectedHeap:CardTableRS
    GenRemSet* rs = SharedHeap::heap()->rem_set();
    assert(rs->rs_kind() == GenRemSet::CardTable, "Wrong rem set kind");
    _rs = (CardTableRS*)rs;// 更新引用
  }
}

template <class T> inline void OopsInGenClosure::do_barrier(T* p) {
  assert(generation()->is_in_reserved(p), "expected ref in generation");
  T heap_oop = oopDesc::load_heap_oop(p);
  assert(!oopDesc::is_null(heap_oop), "expected non-null oop");
  oop obj = oopDesc::decode_heap_oop_not_null(heap_oop);// 解码后的真实地址

  // 指向年轻代（指针< _gen_boundary），标记card
  // If p points to a younger generation, mark the card.
  if ((HeapWord*)obj < _gen_boundary) {
    // 更新rs（记忆集合，结构是cardtable），其实就是更新p所在ct标记
    // CardTableRS
    _rs->inline_write_ref_field_gc(p, obj);
  }
}

template <class T> inline void OopsInGenClosure::par_do_barrier(T* p) {
  assert(generation()->is_in_reserved(p), "expected ref in generation");
  T heap_oop = oopDesc::load_heap_oop(p);
  assert(!oopDesc::is_null(heap_oop), "expected non-null oop");
  oop obj = oopDesc::decode_heap_oop_not_null(heap_oop);
  // If p points to a younger generation, mark the card.
  if ((HeapWord*)obj < gen_boundary()) {
    rs()->write_ref_field_gc_par(p, obj);
  }
}

inline void OopsInKlassOrGenClosure::do_klass_barrier() {
  assert(_scanned_klass != NULL, "Must be");
  _scanned_klass->record_modified_oops();
}

// NOTE! Any changes made here should also be made
// in FastScanClosure::do_oop_work()
template <class T> inline void ScanClosure::do_oop_work(T* p) {
  // p应该就是引用使用的指针（可能是句柄handle，一个对外间接引用）
  T heap_oop = oopDesc::load_heap_oop(p);// java堆中原始oop（可能是压缩指针narrowOop）
  // Should we copy the obj?

  // *p 所引用的那个对象不是NULL，才能进行copy（就等于p引用对象存活需要被copy移动）
  if (!oopDesc::is_null(heap_oop)) {
    // 把p引用的解析成正式使用的oop，其实是压缩指针才需要解析成
    // 需要解析的是narrowOop，（oop时不需要）oopDesc::decode_heap_oop_not_null(narrowOop v)
    oop obj = oopDesc::decode_heap_oop_not_null(heap_oop);

    // _boundary 代表老年代的空间开始地址
    // 只有年轻代对象才会处理
    if ((HeapWord*)obj < _boundary) {
      assert(!_g->to()->is_in_reserved(obj), "Scanning field twice?");
      // 执行copy，得到对象的移动后新内存地址
      oop new_obj = obj->is_forwarded() ? obj->forwardee()
                                        : _g->copy_to_survivor_space(obj);//调用复制
      // 保存copy后新地址到p（应该就是句柄） 
      // 就是p更新指针指向的地址                                 
      oopDesc::encode_store_heap_oop_not_null(p, new_obj);
    }

    // OopsInGenClosure要求执行barrier，为了更新引用后是老年代指向年轻代
    // 其实就是晋升到老年代后，更新指向年轻代的脏卡，barrier就是这个操作需要写屏障

    if (is_scanning_a_klass()) {
      do_klass_barrier();
    } else if (_gc_barrier) {
      // Now call parent closure
      do_barrier(p);
    }
  }
}

inline void ScanClosure::do_oop_nv(oop* p)       { ScanClosure::do_oop_work(p); }
inline void ScanClosure::do_oop_nv(narrowOop* p) { ScanClosure::do_oop_work(p); }

// NOTE! Any changes made here should also be made
// in ScanClosure::do_oop_work()
// 如ScanClosure一样 
template <class T> inline void FastScanClosure::do_oop_work(T* p) {
  T heap_oop = oopDesc::load_heap_oop(p);
  // Should we copy the obj?
  if (!oopDesc::is_null(heap_oop)) {
    oop obj = oopDesc::decode_heap_oop_not_null(heap_oop);
    if ((HeapWord*)obj < _boundary) {
      assert(!_g->to()->is_in_reserved(obj), "Scanning field twice?");
      oop new_obj = obj->is_forwarded() ? obj->forwardee()
                                        : _g->copy_to_survivor_space(obj);
      oopDesc::encode_store_heap_oop_not_null(p, new_obj);
      if (is_scanning_a_klass()) {
        do_klass_barrier();
      } else if (_gc_barrier) {
        // Now call parent closure
        do_barrier(p);
      }
    }
  }
}

inline void FastScanClosure::do_oop_nv(oop* p)       { FastScanClosure::do_oop_work(p); }
inline void FastScanClosure::do_oop_nv(narrowOop* p) { FastScanClosure::do_oop_work(p); }

// Note similarity to ScanClosure; the difference is that
// the barrier set is taken care of outside this closure.
template <class T> inline void ScanWeakRefClosure::do_oop_work(T* p) {
  assert(!oopDesc::is_null(*p), "null weak reference?");
  oop obj = oopDesc::load_decode_heap_oop_not_null(p);
  // weak references are sometimes scanned twice; must check
  // that to-space doesn't already contain this object
  if ((HeapWord*)obj < _boundary && !_g->to()->is_in_reserved(obj)) {
    oop new_obj = obj->is_forwarded() ? obj->forwardee()
                                      : _g->copy_to_survivor_space(obj);
    oopDesc::encode_store_heap_oop_not_null(p, new_obj);
  }
}

inline void ScanWeakRefClosure::do_oop_nv(oop* p)       { ScanWeakRefClosure::do_oop_work(p); }
inline void ScanWeakRefClosure::do_oop_nv(narrowOop* p) { ScanWeakRefClosure::do_oop_work(p); }

#endif // SHARE_VM_MEMORY_GENOOPCLOSURES_INLINE_HPP
