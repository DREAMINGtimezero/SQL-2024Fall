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

#include "executor_delete.h"
#include "system/handle/table_handle.h"
#include "system/handle/index_handle.h"

namespace wsdb {

DeleteExecutor::DeleteExecutor(AbstractExecutorUptr child, TableHandle *tbl, std::list<IndexHandle *> indexes)
    : AbstractExecutor(DML), child_(std::move(child)), tbl_(tbl), indexes_(std::move(indexes)), is_end_(false)
{
    std::vector<RTField> fields(1);
    fields[0] = RTField{.field_ = {.field_name_ = "deleted", .field_size_ = sizeof(int), .field_type_ = TYPE_INT}};
    out_schema_ = std::make_unique<RecordSchema>(fields);
}

void DeleteExecutor::Init() {
    // 不支持 Init 方法
    WSDB_FETAL("DeleteExecutor does not support Init");
}

void DeleteExecutor::Next() {
    if (is_end_) {
        return;  // 已经结束，不再执行
    }

    int count = 0;
    // 删除操作，遍历子执行器的记录并执行删除
    while (!child_->IsEnd()) {
        // 获取下一条记录
        child_->Next();

        auto &record = child_->GetRecord();

        // 执行删除：从表中删除记录
        bool success = tbl_->DeleteRecord(record);

        if (success) {
            // 删除成功后，更新索引
            for (auto *index : indexes_) {
                index->DeleteRecord(record);
            }
            ++count;  // 删除的记录计数
        }
    }

    // 返回删除的记录数量
    std::vector<ValueSptr> values{ValueFactory::CreateIntValue(count)};
    record_ = std::make_unique<Record>(out_schema_.get(), values, INVALID_RID);

    // 标记结束
    is_end_ = true;
}

auto DeleteExecutor::IsEnd() const -> bool {
    return is_end_;  // 如果删除操作已完成，返回 true
}

}  // namespace wsdb
