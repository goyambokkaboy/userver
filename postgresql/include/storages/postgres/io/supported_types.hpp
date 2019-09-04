#pragma once

/// @page pg_types µPg: Supported data types
///
/// µPg provides data type support with a system of buffer parsers and
/// formatters.
/// Please refer to @ref pg_io for more information about the system.
///
/// @par Fundamental PostgreSQL types
///
/// The fundamental PostgreSQL types support is provided by the driver. The
/// table below shows supported Postgres types and their mapping to C++ types
/// provided by the driver. Colum "Default" marks the Postgres type to which
/// a C++ type is mapped when used as a parameter. Where the C++ type is N/A
/// it means that the PosgreSQL data type is not supported. When there is a
/// C++ type in parenthesis, it is a data type that will be supported later
/// and the C++ type is planned counterpart.
/// PG type           | C++ type                                | Default |
/// ----------------- | --------------------------------------- | ------- |
/// smallint          | std::int16_t                            | +       |
/// integer           | std::int32_t                            | +       |
/// bigint            | std::int64_t                            | +       |
/// smallserial       | std::int16_t                            |         |
/// serial            | std::int32_t                            |         |
/// bigserial         | std::int64_t                            |         |
/// boolean           | bool                                    | +       |
/// real              | float                                   | +       |
/// double precision  | double                                  | +       |
/// numeric(p)        | boost::multiprecision::cpp_dec_float<P> | +       |
/// decimal(p)        | boost::multiprecision::cpp_dec_float<P> | +       |
/// money             | N/A                                     |         |
/// text              | std::string                             | +       |
/// char(n)           | std::string                             |         |
/// varchar(n)        | std::string                             |         |
/// "char"            | char                                    | +       |
/// timestamp         | std::chrono::system_clock::time_point   | +       |
/// timestamptz       | std::chrono::system_clock::time_point   | +       |
/// date              | N/A                                     |         |
/// time              | N/A                                     |         |
/// timetz            | N/A                                     |         |
/// interval          | std::chrono::microseconds               |         |
/// bytea             | container of one-byte type              |         |
/// bit(n)            | N/A                                     |         |
/// bit varying(n)    | N/A                                     |         |
/// uuid              | N/A (boost::uuids::uuid)                |         |
///
/// @warning The library doesn't provide support for C++ unsigned integral
/// types intentionally as PostgreSQL doesn't provide unsigned types and
/// using the types with the database is error-prone.
///
/// For more information on timestamps and working with time zones please see
/// @ref pg_timestamp
///
/// @par Arrays
///
/// The driver supports PostgreSQL arrays provided that the element type is
/// supported by the driver. See @ref pg_arrays for more information.
///
/// @par User-defined PostgreSQL types
///
/// The driver provides support for user-defined PostgreSQL types:
/// - domains
/// - enumerations
/// - composite types
///
/// For more information please see @ref pg_user_types.
///
/// @par PostgreSQL types not covered above
///
/// The types not covered above or marked as N/A in the table of fundamental
/// types will be eventually supported later, on request from the driver's
/// users.
///
/// - Range types https://st.yandex-team.ru/TAXICOMMON-372
/// - UUID https://st.yandex-team.ru/TAXICOMMON-373
/// - Bit string https://st.yandex-team.ru/TAXICOMMON-374
/// - Bytea https://st.yandex-team.ru/TAXICOMMON-375
/// - Geometric types https://st.yandex-team.ru/TAXICOMMON-376
/// - Network types https://st.yandex-team.ru/TAXICOMMON-377
/// - JSON types - not planned yet.
///

//@{
/** @name Traits etc */
#include <storages/postgres/io/nullable_traits.hpp>
#include <storages/postgres/io/traits.hpp>
#include <storages/postgres/io/type_mapping.hpp>
#include <storages/postgres/io/type_traits.hpp>
//@}

//@{
/** @name Data types */
#include <storages/postgres/io/boost_multiprecision.hpp>
#include <storages/postgres/io/bytea.hpp>
#include <storages/postgres/io/chrono.hpp>
#include <storages/postgres/io/enum_types.hpp>
#include <storages/postgres/io/floating_point_types.hpp>
#include <storages/postgres/io/integral_types.hpp>
#include <storages/postgres/io/string_types.hpp>
//@}

//@{
/** @name Row types */
#include <storages/postgres/io/row_types.hpp>
//@}

//@{
/** @name Type derivatives */
#include <storages/postgres/io/array_types.hpp>
#include <storages/postgres/io/composite_types.hpp>
#include <storages/postgres/io/optional.hpp>
#include <storages/postgres/io/range_types.hpp>
//@}

//@{
/** @name JSON types */
#include <storages/postgres/io/json_types.hpp>
//@}

//@{
/** @name Stream text IO */
#include <storages/postgres/io/stream_text_formatter.hpp>
#include <storages/postgres/io/stream_text_parser.hpp>
//@}

//@{
/** @name User type registry */
#include <storages/postgres/io/user_types.hpp>
//@}
