// Copyright 2017 Elias Kosunen
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// This file is a part of scnlib:
//     https://github.com/eliaskosunen/scnlib

#pragma once

#include <scn/detail/args.h>
#include <scn/detail/format_string.h>
#include <scn/detail/result.h>
#include <scn/detail/scanner_builtin.h>
#include <scn/detail/vscan.h>

namespace scn {
    SCN_BEGIN_NAMESPACE

    namespace detail {
        template <typename SourceRange, typename ResultRange, typename... Args>
        using scan_result_type = scan_result_tuple<
            std::conditional_t<ranges::borrowed_range<SourceRange>,
                               decltype(detail::map_scan_result_range<
                                        ranges::range_value_t<ResultRange>>(
                                   SCN_DECLVAL(const SourceRange&),
                                   SCN_DECLVAL(
                                       const vscan_result<ResultRange>&))),
                               ranges::dangling>,
            Args...>;
    }

    template <typename SourceRange,
              typename ResultRange,
              typename Context,
              typename... Args>
    detail::scan_result_type<SourceRange, ResultRange, Args...>
    make_scan_result(SourceRange&& source,
                     vscan_result<ResultRange> result,
                     scan_arg_store<Context, Args...>&& args)
    {
        auto result_range =
            detail::map_scan_result_range<ranges::range_value_t<ResultRange>>(
                source, result);
        if (SCN_UNLIKELY(result.error)) {
            return {scan_result{SCN_MOVE(result_range), {}},
                    SCN_MOVE(args.args())};
        }
        return {scan_result{SCN_MOVE(result_range), result.error}, {}};
    }

    namespace detail {
        // Boilerplate for scan()
        template <typename... Args, typename Source, typename Format>
        auto scan_impl(Source&& source,
                       Format format,
                       std::tuple<Args...> args_default_values)
        {
            auto range = scan_map_input_range(source);
            auto args = make_scan_args<decltype(range), Args...>(
                std::move(args_default_values));
            auto result = vscan(range, format, args);
            return make_scan_result(SCN_FWD(source), SCN_MOVE(result),
                                    SCN_MOVE(args));
        }
    }  // namespace detail

    /**
     * Scan Args... from source, according to format.
     */
    template <typename... Args,
              typename Source,
              typename = std::enable_if_t<
                  std::is_same_v<ranges::range_value_t<Source>, char>>>
    SCN_NODISCARD auto scan(Source&& source, format_string<Args...> format)
    {
        return detail::scan_impl<Args...>(SCN_FWD(source), format, {});
    }
    template <typename... Args,
              typename Source,
              typename = std::enable_if_t<
                  std::is_same_v<ranges::range_value_t<Source>, wchar_t>>>
    SCN_NODISCARD auto scan(Source&& source, wformat_string<Args...> format)
    {
        return detail::scan_impl<Args...>(SCN_FWD(source), format, {});
    }

    template <typename... Args,
              typename Source,
              typename = std::enable_if_t<
                  std::is_same_v<ranges::range_value_t<Source>, char>>>
    SCN_NODISCARD auto scan(Source&& source,
                            format_string<Args...> format,
                            std::tuple<Args...>&& args)
    {
        return detail::scan_impl<Args...>(SCN_FWD(source), format,
                                          SCN_MOVE(args));
    }
    template <typename... Args,
              typename Source,
              typename = std::enable_if_t<
                  std::is_same_v<ranges::range_value_t<Source>, wchar_t>>>
    SCN_NODISCARD auto scan(Source&& source,
                            wformat_string<Args...> format,
                            std::tuple<Args...>&& args)
    {
        return detail::scan_impl<Args...>(SCN_FWD(source), format,
                                          SCN_MOVE(args));
    }

    namespace detail {
        template <typename... Args,
                  typename Locale,
                  typename Source,
                  typename Format>
        auto scan_localized_impl(Locale& loc,
                                 Source&& source,
                                 Format format,
                                 std::tuple<Args...> args_default_values)
        {
            auto range = scan_map_input_range(source);
            auto args = make_scan_args<decltype(range), Args...>(
                std::move(args_default_values));
            SCN_CLANG_PUSH_IGNORE_UNDEFINED_TEMPLATE
            auto result = vscan(loc, range, format, args);
            SCN_CLANG_POP_IGNORE_UNDEFINED_TEMPLATE
            return make_scan_result(SCN_FWD(source), SCN_MOVE(result),
                                    SCN_MOVE(args));
        }
    }  // namespace detail

    template <typename... Args,
              typename Locale,
              typename Source,
              typename = std::enable_if_t<
                  std::is_same_v<ranges::range_value_t<Source>, char>>,
              typename = std::void_t<decltype(Locale::classic())>>
    SCN_NODISCARD auto scan(Locale& loc,
                            Source&& source,
                            format_string<Args...> format)
    {
        return detail::scan_localized_impl<Args...>(loc, SCN_FWD(source),
                                                    format, {});
    }
    template <typename... Args,
              typename Locale,
              typename Source,
              typename = std::enable_if_t<
                  std::is_same_v<ranges::range_value_t<Source>, wchar_t>>,
              typename = std::void_t<decltype(Locale::classic())>>
    SCN_NODISCARD auto scan(Locale& loc,
                            Source&& source,
                            wformat_string<Args...> format)
    {
        return detail::scan_localized_impl<Args...>(loc, SCN_FWD(source),
                                                    format, {});
    }

    template <typename... Args,
              typename Locale,
              typename Source,
              typename = std::enable_if_t<
                  std::is_same_v<ranges::range_value_t<Source>, char>>,
              typename = std::void_t<decltype(Locale::classic())>>
    SCN_NODISCARD auto scan(Locale& loc,
                            Source&& source,
                            format_string<Args...> format,
                            std::tuple<Args...>&& args)
    {
        return detail::scan_localized_impl<Args...>(loc, SCN_FWD(source),
                                                    format, args);
    }
    template <typename... Args,
              typename Locale,
              typename Source,
              typename = std::enable_if_t<
                  std::is_same_v<ranges::range_value_t<Source>, wchar_t>>,
              typename = std::void_t<decltype(Locale::classic())>>
    SCN_NODISCARD auto scan(Locale& loc,
                            Source&& source,
                            wformat_string<Args...> format,
                            std::tuple<Args...>&& args)
    {
        return detail::scan_localized_impl<Args...>(loc, SCN_FWD(source),
                                                    format, args);
    }

    namespace detail {
        template <typename Range, typename CharT>
        using context_type_for_impl =
            basic_scan_context<decayed_input_range<Range, CharT>, CharT>;

        template <typename Range>
        using context_type_for =
            detail::context_type_for_impl<Range, ranges::range_value_t<Range>>;

        template <typename T, typename Source>
        auto scan_value_impl(Source&& source, T value)
        {
            auto range = scan_map_input_range(source);
            auto arg =
                detail::make_arg<context_type_for<decltype(range)>>(value);
            auto result = vscan_value(range, arg);
            auto result_range =
                detail::map_scan_result_range(SCN_FWD(source), result);
            return scan_result_type<Source, decltype(result_range), T>{
                scan_result{SCN_MOVE(result_range), result.error},
                std::tuple<T>{SCN_MOVE(value)}};
        }
    }  // namespace detail

    template <typename T, typename Source>
    SCN_NODISCARD auto scan_value(Source&& source)
    {
        return detail::scan_value_impl(SCN_FWD(source), T{});
    }

    template <typename T, typename Source>
    SCN_NODISCARD auto scan_value(Source&& source, T default_value)
    {
        return detail::scan_value_impl(SCN_FWD(source),
                                       std::move(default_value));
    }

#if SCN_USE_IOSTREAMS

    namespace detail {
        scn::istreambuf_view& internal_narrow_stdin();
        scn::wistreambuf_view& internal_wide_stdin();

        template <typename... Args, typename Source, typename Format>
        scan_result_tuple<stdin_range_marker, Args...> input_impl(
            Source& source,
            Format format)
        {
            auto range = scan_map_input_range(source);
            auto args = make_scan_args<decltype(range), Args...>({});
            auto result = vscan_and_sync(range, format, args);
            return make_scan_result(SCN_FWD(source), SCN_MOVE(result),
                                    SCN_MOVE(args));
        }
    }  // namespace detail

    /// Scan Args... from stdin according to format
    /// Prefer this over constructing a view over std::cin, and using scan().
    /// Thread-safe.
    template <typename... Args>
    SCN_NODISCARD auto input(format_string<Args...> format)
    {
        return detail::input_impl<Args...>(detail::internal_narrow_stdin(),
                                           format);
    }
    template <typename... Args>
    SCN_NODISCARD auto input(wformat_string<Args...> format)
    {
        return detail::input_impl<Args...>(detail::internal_wide_stdin(),
                                           format);
    }

    /// Write msg to stdout, and call input<Args...>(format)
    template <typename... Args>
    SCN_NODISCARD auto prompt(const char* msg, format_string<Args...> format)
    {
        std::printf("%s", msg);
        return detail::input_impl<Args...>(detail::internal_narrow_stdin(),
                                           format);
    }
    template <typename... Args>
    SCN_NODISCARD auto prompt(const wchar_t* msg,
                              wformat_string<Args...> format)
    {
        std::wprintf(L"%s", msg);
        return detail::input_impl<Args...>(detail::internal_wide_stdin(),
                                           format);
    }

#endif  // SCN_USE_IOSTREAMS

    SCN_END_NAMESPACE
}  // namespace scn
