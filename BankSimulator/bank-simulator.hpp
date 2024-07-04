#ifndef BANK_SIMULATOR_HPP
#define BANK_SIMULATOR_HPP

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>
#include <memory>
#include <iomanip>
#include <numeric>
#include <cstdarg>

#define get_prob(void) ((float) rand() / (float) RAND_MAX)

namespace BankSimulator
{
    class Customer;
    class Employee;
    class Service;
    class Bank;

    typedef std::vector<std::unique_ptr<Service>> ServiceLayer;

    class Customer
    {
    private: static uint16_t s_number;

    private: uint16_t c_number;

    public:
        Customer(const uint16_t enter_time);

    public:
        void exit(const uint16_t current_time) const noexcept;
    };

    class Employee
    {
    private: std::shared_ptr<Customer> m_current_customer;
             int16_t                   m_start_serving_at;

    public:
        Employee(void);

    public:
        void serve(const std::shared_ptr<Customer>& customer, const uint16_t current_time);
        bool busy(void) const noexcept;
        std::shared_ptr<Customer> done(const uint16_t serve_time, const uint16_t current_time) noexcept;
    };

    class Service
    {
    private: static std::vector<uint16_t> s_weights;

    private: std::string                            c_id;
             uint16_t                               c_weight;
             uint16_t                               c_serve_time;
             std::vector<std::unique_ptr<Employee>> c_employees;

    private: std::queue<std::shared_ptr<Customer>> m_queue;

    private:
        static void choose_service(std::shared_ptr<Customer>& customer, const uint16_t service_number);
        static void enter_customer(uint16_t current_time);

    public:
        Service(const std::string& id,
                const uint16_t     weight,
                const uint16_t     serve_time,
                const size_t       employees_count);

    public:
        void print(void) const noexcept;
        void enter(const std::shared_ptr<Customer>& customer);
        void serve_from_queue(const uint16_t current_time);
        void checkout_done_customers(const uint16_t current_time, const uint16_t next);
        uint16_t get_weight(void) const noexcept;
    
    friend Bank;
    };

    class Bank
    {
    private: static std::vector<ServiceLayer> s_services_layers;
             static uint16_t                  s_current_time;

    private:
        static void add_layer(Service* service, ...);
        static void loop_all(const uint16_t current_time);
        static void print_all(void) noexcept;

    public:
        static void start_working(const uint16_t working_time);
        static ServiceLayer const& get_layer(const size_t index);
    };
}
#endif // BANK_SIMULATOR_HPP