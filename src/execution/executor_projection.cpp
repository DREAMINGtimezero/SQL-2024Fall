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

#include "executor_projection.h"

namespace wsdb {

ProjectionExecutor::ProjectionExecutor(AbstractExecutorUptr child, RecordSchemaUptr proj_schema)
    : AbstractExecutor(Basic), child_(std::move(child)) {
    out_schema_ = std::move(proj_schema);
}

void ProjectionExecutor::Init() {
    // 初始化时，调用子执行器的 Init 方法进行初始化
    child_->Init();
}

void ProjectionExecutor::Next() {
    if (IsEnd()) {
        WSDB_FETAL("ProjectionExecutor is already at the end.");
    }

    // 获取子执行器的下一条记录
    child_->Next();
    
    // 根据投影模式创建新的记录
    record_ = std::make_unique<Record>(out_schema_.get(), *child_->GetRecord());

    // 这里需要确保调用子执行器时，调用它的 `GetRecord()` 获取子记录
}

auto ProjectionExecutor::IsEnd() const -> bool {
    // 检查子执行器是否结束
    return child_->IsEnd();
}

}  // namespace wsdb
