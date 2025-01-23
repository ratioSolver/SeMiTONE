#pragma once

#include "bool.hpp"
#include "integer.hpp"
#include "rational.hpp"
#include <memory>
#include <vector>

namespace semitone
{
  class context;

  class var
  {
  public:
    var(const context &ctx);
    virtual ~var() = default;

    [[nodiscard]] const context &get_ctx() const noexcept;

  private:
    const context &ctx;
  };

  using expr = std::shared_ptr<var>;

  class bool_var : public var
  {
  public:
    bool_var(const context &ctx);
    bool_var(const context &ctx, utils::lbool val);
    virtual ~bool_var() = default;

    [[nodiscard]] virtual utils::lbool val() const noexcept;

  private:
    utils::lbool value;
  };

  using bool_expr = std::shared_ptr<bool_var>;

  class and_expr final : public bool_var
  {
  public:
    and_expr(const context &ctx, std::vector<bool_expr> &&args);

    [[nodiscard]] const std::vector<bool_expr> &args() const noexcept;

    [[nodiscard]] utils::lbool val() const noexcept override;

  private:
    std::vector<bool_expr> arguments;
  };

  class or_expr final : public bool_var
  {
  public:
    or_expr(const context &ctx, std::vector<bool_expr> &&args);

    [[nodiscard]] const std::vector<bool_expr> &args() const noexcept;

    [[nodiscard]] utils::lbool val() const noexcept override;

  private:
    std::vector<bool_expr> arguments;
  };

  class not_expr final : public bool_var
  {
  public:
    not_expr(const context &ctx, bool_expr arg);

    [[nodiscard]] bool_expr arg() const noexcept;

    [[nodiscard]] utils::lbool val() const noexcept override;

  private:
    bool_expr argument;
  };

  class int_var : public var
  {
  public:
    int_var(const context &ctx);
    int_var(const context &ctx, utils::integer val);
    int_var(const context &ctx, utils::integer val, utils::integer lb, utils::integer ub);
    virtual ~int_var() = default;

    [[nodiscard]] virtual utils::integer lb() const noexcept;
    [[nodiscard]] virtual utils::integer ub() const noexcept;

    [[nodiscard]] virtual utils::integer val() const noexcept;

  private:
    utils::integer value, lower_bound, upper_bound;
  };

  using int_expr = std::shared_ptr<int_var>;

  class int_sum final : public int_var
  {
  public:
    int_sum(const context &ctx, std::vector<int_expr> &&args);

    [[nodiscard]] const std::vector<int_expr> &args() const noexcept;

    [[nodiscard]] utils::integer lb() const noexcept override;
    [[nodiscard]] utils::integer ub() const noexcept override;

    [[nodiscard]] utils::integer val() const noexcept override;

  private:
    std::vector<int_expr> arguments;
  };

  class real_var : public var
  {
  public:
    real_var(const context &ctx);
    real_var(const context &ctx, utils::rational val);
    real_var(const context &ctx, utils::rational val, utils::rational lb, utils::rational ub);
    virtual ~real_var() = default;

    [[nodiscard]] utils::rational lb() const noexcept;
    [[nodiscard]] utils::rational ub() const noexcept;

    [[nodiscard]] utils::rational val() const noexcept;

  private:
    utils::rational value, lower_bound, upper_bound;
  };

  using real_expr = std::shared_ptr<real_var>;

  class string_var : public var
  {
  public:
    string_var(const context &ctx);
    string_var(const context &ctx, std::string val);
    virtual ~string_var() = default;

    [[nodiscard]] std::string val() const noexcept;

  private:
    std::string value;
  };

  using string_expr = std::shared_ptr<string_var>;
} // namespace semitone
