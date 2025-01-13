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

#include "executor_insert.h"
#include "system/handle/record_handle.h"
#include "system/handle/index_handle.h"
#include "common/value.h"

namespace wsdb {

InsertExecutor::InsertExecutor(TableHandle *tbl, std::list<IndexHandle *> indexes, std::vector<RecordUptr> inserts)
    : AbstractExecutor(DML), tbl_(tbl), indexes_(std::move(indexes)), inserts_(std::move(inserts)), is_end_(false)
{
    // Initialize the output schema. We are adding a single field "inserted" that will be the output of the executor.
    std::vector<RTField> fields(1);
    fields[0] = RTField{.field_ = {.field_name_ = "inserted", .field_size_ = sizeof(int), .field_type_ = TYPE_INT}};
    out_schema_ = std::make_unique<RecordSchema>(fields);
}

void InsertExecutor::Init() {
    // InsertExecutor does not require any initialization beyond its constructor
    // This function is intentionally left empty
    // If necessary, initialization logic (e.g., setting up the table or indexes) could go here
}

void InsertExecutor::Next() {
    // Insert a record into the table and update indexes.
    if (inserts_.empty()) {
        // If there are no records to insert, set `is_end_` to true and return.
        is_end_ = true;
        return;
    }

    // Get the first record to insert.
    auto record_to_insert = std::move(inserts_.front());
    inserts_.erase(inserts_.begin()); // Remove the inserted record from the list

    // Insert the record into the table using TableHandle's InsertRecord method (or equivalent)
    RID rid = tbl_->InsertRecord(record_to_insert.get()); // Insert the record and get its RID.
    if (rid == INVALID_RID) {
        // If insertion failed, print an error or handle it as needed.
        WSDB_FETAL("Insert failed for record: {}", record_to_insert.get());
    }

    // Now insert the record into all the indexes.
    for (auto index : indexes_) {
        index->InsertRecord(record_to_insert.get(), rid);
    }

    // Set the count of inserted records (for output schema).
    int count = 1;

    // Create a new record to return the insertion result, with a single field ("inserted") holding the count.
    std::vector<ValueSptr> values{ValueFactory::CreateIntValue(count)};
    record_ = std::make_unique<Record>(out_schema_.get(), values, INVALID_RID); // Create a result record.
}

auto InsertExecutor::IsEnd() const -> bool {
    return is_end_;
}

}  // namespace wsdb
