/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Wangyunlai on 2022/5/22.
//

#include "sql/stmt/update_stmt.h"

UpdateStmt::UpdateStmt(Table *table, Value *values, int value_amount)
    : table_(table), values_(values), value_amount_(value_amount)
{}

RC UpdateStmt::create(Db *db, const UpdateSqlNode &update_sql, Stmt *&stmt)
{
  // 1. 查找表是否存在
  Table *table = db->find_table(update_sql.relation_name.c_str());
  if (table == nullptr) {
    LOG_WARN("Table not found: %s", update_sql.relation_name.c_str());
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  // 2. 检查字段是否存在
  const TableMeta &table_meta = table->table_meta();
  const FieldMeta *field_meta = table_meta.field(update_sql.attribute_name.c_str());
  if (field_meta == nullptr) {
    LOG_WARN("Field not found: %s in table %s", 
             update_sql.attribute_name.c_str(), table->name());
    return RC::SCHEMA_FIELD_NOT_EXIST;
  }

  // 3. 检查更新值类型是否匹配字段类型
  const Value &value = update_sql.value;
  if (!check_value_type_match(field_meta->type(), value.type())) {
    LOG_WARN("Value type mismatch for field %s", field_meta->name());
    return RC::TYPE_MISMATCH;
  }

  // 4. 构造UpdateStmt对象（仅支持单字段更新）
  Value *values = new Value[1];
  values[0] = value;  // 拷贝待更新的值
  stmt = new UpdateStmt(table, values, 1);
  return RC::SUCCESS;
}

// 辅助函数：检查值类型是否与字段类型匹配
bool check_value_type_match(AttrType field_type, AttrType value_type) {
  // 处理int/float等兼容类型，根据实际支持的类型扩展
  if (field_type == value_type) return true;
  if ((field_type == AttrType::INT && value_type == AttrType::FLOAT) ||
      (field_type == AttrType::FLOAT && value_type == AttrType::INT)) {
    return true;  // 允许int和float互转（可选，根据需求调整）
  }
  return false;
}
