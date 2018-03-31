////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2016 Husky Team
/// Copyright 2018 ArangoDB GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Simon Grätzer
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGODB_HYDRA_OBJLIST_H
#define ARANGODB_HYDRA_OBJLIST_H 1

#include "boost/random.hpp"

namespace arangodb {
namespace hydra {

// Common superclass for DataFrame, mostly for keeping pointer references

class DataFrameBase /*: public ChannelSource, public ChannelDestination*/ {
public:
  DataFrameBase() : _id(s_counter) { s_counter++; }
  
  virtual ~DataFrameBase() = default;
  
  DataFrameBase(const DataFrameBase&) = delete;
  DataFrameBase& operator=(const DataFrameBase&) = delete;
  
  DataFrameBase(DataFrameBase&&) = delete;
  DataFrameBase& operator=(DataFrameBase&&) = delete;
  
  inline size_t id() const { return _id; }
  
  virtual size_t size() const = 0;
  
private:
  size_t _id;
  
  static thread_local size_t s_counter;
};

template <typename ObjT>
class DataFrame : public DataFrameBase {
public:
  // TODO(all): should be protected. The list should be constructed by possibly Context
  DataFrame() = default;
  
  virtual ~DataFrame() {
    /*for (auto& it : this->attrlist_map)
      delete it.second;
    this->attrlist_map.clear();*/
  }
  
  DataFrame(const DataFrame&) = default;
  DataFrame& operator=(const DataFrame&) = default;
  
  DataFrame(DataFrame&&) = default;
  DataFrame& operator=(DataFrame&&) = default;
  
  inline bool isSorted() const { return _sortedSize > 0; }
  inline size_t getSortedSize() const { return _sortedSize; }
  inline size_t getNumDeleted() const { return _numDeleted; }
  //inline size_t get_hashed_size() const { return hashed_objs_.size(); }
  //inline size_t get_size() const override { return objlist_data_.get_size(); }
  //inline size_t get_vector_size() const { return objlist_data_.get_vector_size(); }
  
  
  inline ObjT& get(size_t i) { return _data[i]; }
  
  virtual size_t size() const = 0;
  
  // Sort the objlist
  void sort() {
    auto& data = objlist_data_.data_;
    if (data.size() == 0)
      return;
    /*std::vector<int> order(this->get_size());
    for (int i = 0; i < order.size(); ++i)
      order[i] = i;
    // sort the permutation
    std::sort(order.begin(), order.end(),
              [&](const size_t a, const size_t b) { return data[a].key() < data[b].key(); });
    // apply the permutation on all the attribute lists
    for (auto& it : this->attrlist_map)
      it.second->reorder(order);*/
    std::sort(data.begin(), data.end(), [](ObjT const& a, ObjT const& b) { return a.key() < b.key(); });
    _sortedSize = _data.size();
    _objMap.clear();
  }
  
  // Shuffle the objlist, only shuffle the locations of objects
  /*void shuffle() {
    //_data.shuffle();
    
    _sortedSize = 0;
    _deletedBitmap.clear();
    _objMap.clear();
  }*/
  
  // remove deleted objects
  void compactList() {
    if (_data.size() == 0)
      return;
    size_t i = 0, j;
    // move i to the first empty place
    while (i < _data.size() && !_deletedBitmap[i])
      i++;
    
    if (i == data.size())
      return;
    
    for (j = _data.size() - 1; j > 0; j--) {
      if (!_deletedBitmap[j]) {
        _data[i] = std::move(_data[j]);
        // move j_th attribute to i_th for all attr lists
        /*for (auto& it : this->attrlist_map)
          it.second->move(i, j);*/
        i += 1;
        // move i to the next empty place
        while (i < _data.size() && !_deletedBitmap[i])
          i++;
      }
      if (i >= j)
        break;
    }
    _data.resize(j);
    _deletedBitmap.resize(j);
    /*for (auto& it : this->attrlist_map)
      it.second->resize(j);*/
    _numDeleted = 0;
    std::fill(_deletedBitmap.begin(), _deletedBitmap.end(), 0);
  }
  
  // Find obj according to key
  // @Return a pointer to obj
  ObjT* find(const typename ObjT::KeyT& key) {
    if (!_sorted) {
      return nullptr;
    }
    
    if (_data.size() == 0)
      return nullptr;
    ObjT* start_addr = _data.data();
    int r = this->_sortedSize - 1;
    int l = 0;
    int m = (r + l) / 2;
    
    while (l <= r) {
      // __builtin_prefetch(start_addr+(m+1+r)/2, 0, 1);
      // __builtin_prefetch(start_addr+(l+m-1)/2, 0, 1);
#ifdef ENABLE_LIST_FIND_PREFETCH
      __builtin_prefetch(&(start_addr[(m + 1 + r) / 2].key()), 0, 1);
      __builtin_prefetch(&(start_addr[(l + m - 1) / 2].key()), 0, 1);
#endif
      // __builtin_prefetch(&working_list[(m+1+r)/2], 0, 1);
      // __builtin_prefetch(&working_list[(l+m-1)/2], 0, 1);
      auto tmp = start_addr[m].key();
      if (tmp == key)
        return &working_list[m];
      else if (tmp < key)
        l = m + 1;
      else
        r = m - 1;
      m = (r + l) / 2;
    }
    
    // The object to find is not in the sorted part
    if ((_sortedSize < _data.size())) {
      auto it = _objMap.find(key);
      if (it != _objMap.end()) {
        return &(_data[it->second]);
      }
    }
    return nullptr;
  }
  
  // Find the index of an obj
  size_t indexOf(const ObjT* const ptr) const {
    ptrdiff_t diff = ptr - _data.data();
    if (ptr < _data.data() || diff > _data.size()) {
      THROW_ARANGO_EXCEPTION(TRI_ERROR_INTERNAL);
    }
    return static_cast<size_t>(diff);
  }
  
  // Add an object
  size_t add(const ObjT& obj) {
    size_t ret = _objMap[obj.key()] = _data.size();
    _data.push_back(obj);
    _deletedBitmap.push_back(0);
    return ret;
  }
  
  size_t add(ObjT&& obj) {
    size_t ret = _objMap[obj.key()] = _data.size();
    _data.push_back(std::move(obj));
    _deletedBitmap.push_back(0);
    return ret;
  }
  
  // Delete an object
  size_t erase(const ObjT* const ptr) {
    // TODO(all): Decide whether we can remove this
    // if (unlikely(del_bitmap_.size() < data.size())) {
    //     del_bitmap_.resize(data.size());
    // }
    // lazy operation
    ptrdiff_t idx = ptr - _data.data();
    if (ptr < _data.data() || idx >= _data.size()) {
      THROW_ARANGO_EXCEPTION(TRI_ERROR_INTERNAL);
    }
    if (_deletedBitmap.size() <= idx) {
      _deletedBitmap.resize(idx + 1);
    }
    _deletedBitmap[idx] = true;
    _numDeleted++;
    return idx;
  }
  
  // Check a certain position of del_bitmap_
  // @Return True if it's deleted
  bool isDeleted(size_t idx) const {
    if (_deletedBitmap.size() <= idx) {
      return false;
    }
    return _deletedBitmap[idx]; }
  
  void clear() {
    _sortedSize = 0;
    _data.clear();
    _deletedBitmap.clear();
    _objMap.clear();
  }
  
  /*class iterator : std::iterator<std::output_iterator_tag, ObjT> {
    
  };
  
  std::vector<ObjT>::iterator begin() const {
    return _data.begin();
  }
  
  std::vector<ObjT>::iterator end() const {
    return _data.end();
  }*/
  
  // Create AttrList
  /*template <typename AttrT>
  AttrList<ObjT, AttrT>& create_attrlist(const std::string& attr_name, const AttrT& default_attr = {}) {
    if (attrlist_map.find(attr_name) != attrlist_map.end())
      throw base::HuskyException("ObjList<T>::create_attrlist error: name already exists");
    auto* attrlist = new AttrList<ObjT, AttrT>(&objlist_data_, default_attr);
    attrlist_map.insert({attr_name, attrlist});
    return (*attrlist);
  }
  
  // Get AttrList
  template <typename AttrT>
  AttrList<ObjT, AttrT>& get_attrlist(const std::string& attr_name) {
    if (attrlist_map.find(attr_name) == attrlist_map.end())
      throw base::HuskyException("ObjList<T>::get_attrlist error: AttrList does not exist");
    return (*static_cast<AttrList<ObjT, AttrT>*>(attrlist_map[attr_name]));
  }
  
  // Delete AttrList
  size_t del_attrlist(const std::string& attr_name) {
    if (attrlist_map.find(attr_name) != attrlist_map.end())
      delete attrlist_map[attr_name];
    return attrlist_map.erase(attr_name);
  }
  
  void migrate_attribute(BinStream& bin, const size_t idx) {
    if (!this->attrlist_map.empty())
      for (auto& item : this->attrlist_map)
        item.second->migrate(bin, idx);
  }
  
  void process_attribute(BinStream& bin, const size_t idx) {
    if (!this->attrlist_map.empty())
      for (auto& item : this->attrlist_map)
        item.second->process_bin(bin, idx);
  }*/


  
  /*bool write_to_disk() {
    DiskStore ds(id2str());
    BinStream bs;
    deletion_finalize();
    sort();
    bs << objlist_data_;
    this->clear_from_memory();
    return ds.write(std::move(bs));
  }
  
  void read_from_disk(const std::string& objlist_path) {
    DiskStore ds(objlist_path);
    BinStream bs = ds.read();
    objlist_data_.clear();
    bs >> objlist_data_;
    sorted_size_ = objlist_data_.data_.size();
    del_bitmap_.clear();
    del_bitmap_.resize(sorted_size_, false);
    hashed_objs_.clear();
  }*/
  
  
  /*size_t estimated_storage_size(const double sample_rate = 0.005) {
    if (this->get_vector_size() == 0)
      return 0;
    const size_t sample_num = this->get_vector_size() * sample_rate + 1;
    BinStream bs;
    
    // sample
    std::unordered_set<size_t> sample_container;
    boost::random::mt19937 generator;
    boost::random::uniform_real_distribution<double> distribution(0.0, 1.0);
    while (sample_container.size() < sample_num) {
      size_t index = distribution(generator) * objlist_data_.get_vector_size();
      sample_container.insert(index);
    }
    
    // log the size
    for (auto iter = sample_container.begin(); iter != sample_container.end(); ++iter)
      bs << objlist_data_.data_[*iter];
    
    std::vector<ObjT>& v = objlist_data_.data_;
    size_t ret = bs.size() * sizeof(char) * v.capacity() / sample_num;
    return ret;
  }*/
  
  
private:
  size_t _sortedSize = 0;
  size_t _numDeleted = 0;
  std::vector<ObjT> _data;
  std::vector<bool> _deletedBitmap;
  std::unordered_map<typename ObjT::KeyT, size_t> _objMap;
  //std::unordered_map<std::string, AttrListBase*> attrlist_map;
};
  
/*template <>
class DataFrame<velocypack::Slice> : public DataFrameBase {
  
  
  inline bool isSorted() const { return false; }

  
private:
  velocypack::Buffer<uint8_t> _buffer;
  size_t _size;
};*/
  
}
}
#endif
