#include <gtest/gtest.h>
#include <fraction.h>
#include <client_logger.h>
#include <client_logger_builder.h>
#include <sstream>

logger *create_logger(
    std::vector<std::pair<std::string, logger::severity>> const &output_file_streams_setup,
    bool use_console_stream = true,
    logger::severity console_stream_severity = logger::severity::debug)
{
    logger_builder *builder = new client_logger_builder();
    
    if (use_console_stream)
    {
        builder->add_console_stream(console_stream_severity);
    }
    
    for (auto &output_file_stream_setup: output_file_streams_setup)
    {
        builder->add_file_stream(output_file_stream_setup.first, output_file_stream_setup.second);
    }
    
    logger *built_logger = builder->build();
    
    delete builder;
    
    return built_logger;
}

TEST(fraction_positive_tests, addition)
{
    logger *logger = create_logger(std::vector<std::pair<std::string, logger::severity>>{});
    
    fraction a(1, 2);

    fraction b(1, 3);

    fraction result = a + b;
    
    EXPECT_EQ(result.to_string(), "5/6");
    
    delete logger;
}

TEST(fraction_positive_tests, subtraction)
{
    logger *logger = create_logger(std::vector<std::pair<std::string, logger::severity>>{});
    
    fraction a(3, 4);
    fraction b(1, 4);
    fraction result = a - b;
    
    EXPECT_EQ(result.to_string(), "1/2");
    
    delete logger;
}

TEST(fraction_positive_tests, multiplication)
{
    logger *logger = create_logger(std::vector<std::pair<std::string, logger::severity>>{});
    
    fraction a(2, 3);
    fraction b(3, 4);
    fraction result = a * b;
    
    EXPECT_EQ(result.to_string(), "1/2");
    
    delete logger;
}

TEST(fraction_positive_tests, division)
{
    logger *logger = create_logger(std::vector<std::pair<std::string, logger::severity>>{});
    
    fraction a(5, 6);
    fraction b(2, 3);
    fraction result = a / b;
    
    EXPECT_EQ(result.to_string(), "5/4");
    
    delete logger;
}

TEST(fraction_optimization, reduce)
{
    logger *logger = create_logger(std::vector<std::pair<std::string, logger::severity>>{});
    
    fraction a(4, 6);
    
    EXPECT_EQ(a.to_string(), "2/3");
    
    delete logger;
}

TEST(fraction_comparison, equality)
{
    logger *logger = create_logger(std::vector<std::pair<std::string, logger::severity>>{});
    
    fraction a(2, 4);
    fraction b(1, 2);
    
    EXPECT_TRUE(a == b);
    
    delete logger;
}

TEST(fraction_comparison, less_than)
{
    logger *logger = create_logger(std::vector<std::pair<std::string, logger::severity>>{});
    
    fraction a(1, 3);
    fraction b(1, 2);
    
    EXPECT_TRUE(a < b);
    
    delete logger;
}

TEST(fraction_exception, division_by_zero)
{
    logger *logger = create_logger(std::vector<std::pair<std::string, logger::severity>>{});
    
    fraction a(1, 2);
    fraction b(0, 1);
    
    EXPECT_THROW(a / b, std::invalid_argument);
    
    delete logger;
}

TEST(fraction_unary, negation)
{
    logger *logger = create_logger(std::vector<std::pair<std::string, logger::severity>>{});
    
    fraction a(3, 4);
    fraction b = -a;
    
    EXPECT_EQ(b.to_string(), "-3/4");
    
    delete logger;
}

TEST(fraction_unary, absolute)
{
    logger *logger = create_logger(std::vector<std::pair<std::string, logger::severity>>{});
    
    fraction a(-3, 4);
    fraction result = a.abs();
    
    EXPECT_EQ(result.to_string(), "3/4");
    
    delete logger;
}

TEST(fraction_io, input_output)
{
    logger *logger = create_logger(std::vector<std::pair<std::string, logger::severity>>{});
    
    fraction f;
    std::istringstream iss("5/7");
    iss >> f;
    
    std::ostringstream oss;
    oss << f;
    
    EXPECT_EQ(oss.str(), "5/7");
    
    delete logger;
}

TEST(fraction_io, integer_input)
{
    logger *logger = create_logger(std::vector<std::pair<std::string, logger::severity>>{});
    
    fraction f;
    std::istringstream iss("8");
    iss >> f;
    
    EXPECT_EQ(f.to_string(), "8");
    
    delete logger;
}

TEST(fraction_trigonometry, sin_zero)
{
    logger *logger = create_logger(std::vector<std::pair<std::string, logger::severity>>{});
    
    fraction angle(0);
    fraction result = angle.sin(fraction(1, 1000000));
    
    EXPECT_EQ(result.to_string(), "0");
    
    delete logger;
}

TEST(fraction_trigonometry, cos_zero)
{
    logger *logger = create_logger(std::vector<std::pair<std::string, logger::severity>>{});
    
    fraction angle(0);
    fraction result = angle.cos(fraction(1, 1000000));
    
    EXPECT_EQ(result.to_string(), "1");
    
    delete logger;
}

TEST(fraction_power, square)
{
    logger *logger = create_logger(std::vector<std::pair<std::string, logger::severity>>{});
    
    fraction a(3, 2);
    fraction result = a.pow(2);
    
    EXPECT_EQ(result.to_string(), "9/4");
    
    delete logger;
}

TEST(fraction_log, ln_one)
{
    logger *logger = create_logger(std::vector<std::pair<std::string, logger::severity>>{});
    
    fraction a(1);
    fraction result = a.ln(fraction(1, 1000000));
    
    EXPECT_EQ(result.to_string(), "0");
    
    delete logger;
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(fraction_inverse_trig, arccos_basic)
{
    logger *logger = create_logger(std::vector<std::pair<std::string, logger::severity>>{});
    
    fraction x(0);
    fraction pi_2 = fraction(1).arcsin(fraction(1, 1000)) * fraction(2);
    
    // arccos(0) = π/2
    fraction result = x.arccos(fraction(1, 1000));
    fraction expected = pi_2;
    EXPECT_TRUE((result - expected).abs() <= fraction(1, 1000));
    
    delete logger;
}

TEST(fraction_inverse_trig, arcsec_edge_cases)
{
    logger *logger = create_logger(std::vector<std::pair<std::string, logger::severity>>{});

    // arcsec(-1) = π
    fraction x2(-1);
    fraction pi = fraction(1).arcsin(fraction(1, 1000)) * fraction(2) * fraction(2);
    fraction result2 = x2.arcsec(fraction(1, 1000));
    EXPECT_TRUE((result2 - pi).abs() <= fraction(1, 1000));
    
    delete logger;
}

TEST(fraction_inverse_trig, arccosec_exception)
{
    logger *logger = create_logger(std::vector<std::pair<std::string, logger::severity>>{});
    
    fraction x(1, 2);
    EXPECT_THROW(x.arccosec(fraction(1, 1000)), std::invalid_argument);
    
    delete logger;
}

TEST(fraction_inverse_trig, arcsec_arccos_relation)
{
    logger *logger = create_logger(std::vector<std::pair<std::string, logger::severity>>{});
    
    fraction x(2);
    fraction result_arcsec = x.arcsec(fraction(1, 1000));
    fraction result_arccos = (fraction(1)/x).arccos(fraction(1, 1000));
    
    EXPECT_TRUE((result_arcsec - result_arccos).abs() <= fraction(1, 1000));
    
    delete logger;
}

TEST(fraction_inverse_trig, arccosec_arcsin_relation)
{
    logger *logger = create_logger(std::vector<std::pair<std::string, logger::severity>>{});
    
    fraction x(3);
    fraction result_arccosec = x.arccosec(fraction(1, 1000));
    fraction result_arcsin = (fraction(1)/x).arcsin(fraction(1, 1000));
    
    EXPECT_TRUE((result_arccosec - result_arcsin).abs() <= fraction(1, 1000));
    
    delete logger;
}