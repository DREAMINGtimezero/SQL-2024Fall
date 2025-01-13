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
// Created by ziqi on 2024/7/18.
//

#include "executor_filter.h"

namespace wsdb {

FilterExecutor::FilterExecutor(AbstractExecutorUptr child, std::function<bool(const Record &)> filter)
    : AbstractExecutor(Basic), child_(std::move(child)), filter_(std::move(filter))
{
}

void FilterExecutor::Init() {
    // 不需要初始化操作，可以为空
}

void FilterExecutor::Next() {
    // 循环获取记录直到找到符合过滤条件的记录
    while (!child_->IsEnd()) {
        child_->Next();  // 获取子执行器的下一条记录

        auto &record = child_->GetRecord();  // 获取当前记录

        if (filter_(record)) {
            // 如果记录符合过滤条件，则返回该记录
            record_ = std::make_unique<Record>(child_->GetOutSchema(), record);
            return;
        }
    }

    // 如果没有符合条件的记录，标记为结束
    record_ = nullptr;
}

auto FilterExecutor::IsEnd() const -> bool {
    return record_ == nullptr;  // 如果 record_ 为 nullptr，表示已经没有符合条件的记录
}

auto FilterExecutor::GetOutSchema() const -> const RecordSchema * {
    return child_->GetOutSchema();  // 输出模式与子执行器相同
}

}  // namespace wsdb
