/*------------------------------------------------------------------------------
 - Copyright (c) 2024. Websoft research group, Nanjing University.
 -
 - This program is free software: you can redistribute it and/or modify
 - it under the terms of the GNU General Public License as published by
 - the Free Software Foundation, either version 3 of the License, or
 - (at your option) any later version.
 -
 - This program is distributed in the hope that it will be useful,
 - but WITHOUT ANY WARRANTY; without even the implied warranty of
 - MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 - GNU General Public License for more details.
 -
 - You should have received a copy of the GNU General Public License
 - along with this program.  If not, see <https://www.gnu.org/licenses/>.
 -----------------------------------------------------------------------------*/

//
// Created by ziqi on 2024/7/17.
//
#include "lru_replacer.h"
#include "common/config.h"
#include "../common/error.h"
#include <list>
#include <unordered_map>
#include <iostream>  

namespace wsdb {

LRUReplacer::LRUReplacer() : cur_size_(0), max_size_(BUFFER_POOL_SIZE), timestamp_counter_(0) {}

auto LRUReplacer::Victim(frame_id_t *frame_id) -> bool {
    std::lock_guard<std::mutex> guard(latch_);
    
    if (cur_size_ == 0) {
        return false;
    }

    size_t max_dist = 0;  // 初始化最大时间戳差
    frame_id_t victim = INVALID_FRAME_ID;

    // 遍历 LRU 列表中的所有帧
    for (auto it = lru_list_.begin(); it != lru_list_.end(); ++it) {
        // 确保该帧是未被钉住的
        if (!it->second) {  // false 表示未钉住
            size_t dist = timestamp_counter_ - access_timestamp_[it->first];
            
            std::cout << "Checking frame_id: " << it->first 
                      << ", access_timestamp: " << access_timestamp_[it->first] 
                      << ", dist: " << dist << std::endl;
            
            // 选择 `dist` 最大的帧
            if (dist >= max_dist) {
                max_dist = dist;
                victim = it->first;
            }
        }
    }

    if (victim != INVALID_FRAME_ID) {
        *frame_id = victim;
        lru_list_.remove_if([victim](const std::pair<frame_id_t, bool>& elem) { return elem.first == victim; });
        lru_hash_.erase(victim);
        --cur_size_;
        
        std::cout << "Victim selected: " << victim << std::endl;

        return true;
    }

    return false;
}


void LRUReplacer::Pin(frame_id_t frame_id) {
    std::lock_guard<std::mutex> guard(latch_);
    
    auto it = lru_hash_.find(frame_id);
    if (it != lru_hash_.end()) {
        // 如果帧已被钉住，移除它并减小当前大小
        lru_list_.erase(it->second);
        lru_hash_.erase(it);
        --cur_size_;
    }
}

void LRUReplacer::Unpin(frame_id_t frame_id) {
    std::lock_guard<std::mutex> guard(latch_);
    
    auto it = lru_hash_.find(frame_id);
    if (it == lru_hash_.end()) {
        // 如果帧不在列表中，则将其添加到 LRU 列表
        lru_list_.push_back({frame_id, false});  // false 表示未钉住
        lru_hash_[frame_id] = std::prev(lru_list_.end());
        
        // 先记录时间戳，再增加时间戳计数器
        access_timestamp_[frame_id] = timestamp_counter_;
        ++cur_size_;
        ++timestamp_counter_; // 增加时间戳计数器
        
        std::cout << "Unpinning new frame_id: " << frame_id 
                  << ", timestamp: " << access_timestamp_[frame_id] << std::endl;
    } else {
        // 如果帧已经在列表中，则更新它的时间戳
        access_timestamp_[frame_id] = timestamp_counter_;
        ++timestamp_counter_;
        std::cout << "Updating frame_id: " << frame_id 
                  << ", new timestamp: " << access_timestamp_[frame_id] << std::endl;
    }
}

auto LRUReplacer::Size() -> size_t {
    std::lock_guard<std::mutex> guard(latch_);
    return cur_size_;
}

}  // namespace wsdb
