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

#include "executor_seqscan.h"

namespace wsdb {

SeqScanExecutor::SeqScanExecutor(TableHandle *tab) : AbstractExecutor(Basic), tab_(tab), rid_(INVALID_RID) {}

void SeqScanExecutor::Init()
{
    // Initialize the scan with the first record ID
    rid_ = tab_->GetFirstRID();

    if (rid_ == INVALID_RID) {
        // If there are no records in the table, set is_end_ to true immediately
        is_end_ = true;
    } else {
        is_end_ = false;
    }
}

void SeqScanExecutor::Next()
{
    if (is_end_) {
        WSDB_FETAL("SeqScanExecutor has already finished scanning");
    }

    // Get the next record ID in the table
    rid_ = tab_->GetNextRID(rid_);

    // If we reach INVALID_RID, the scan is complete
    if (rid_ == INVALID_RID) {
        is_end_ = true;
    } else {
        // Otherwise, load the current record for output
        auto record = tab_->GetRecord(rid_);
        record_ = std::make_unique<Record>(tab_->GetSchema(), record);
    }
}

auto SeqScanExecutor::IsEnd() const -> bool
{
    return is_end_;
}

auto SeqScanExecutor::GetOutSchema() const -> const RecordSchema *
{
    return &tab_->GetSchema();
}

}  // namespace wsdb
