#include "bank-simulator.hpp"

#pragma region BankSimulator::Customer
uint16_t BankSimulator::Customer::s_number = 0;

BankSimulator::Customer::Customer(const uint16_t enter_time)
    : c_number(s_number++)
{
    std::cout << "+ Customer [" << c_number << "] entered at: " << enter_time << "\n\n";
}

void
BankSimulator::Customer::exit(const uint16_t current_time)
const noexcept
{
    std::cout << "- Customer [" << c_number << "] exited at: " << current_time << "\n\n";
}
#pragma endregion // BankSimulator::Customer

#pragma region BankSimulator::Employee
BankSimulator::Employee::Employee(void)
    : m_current_customer(nullptr),
      m_start_serving_at(-1)
{ }

void
BankSimulator::Employee::serve(const std::shared_ptr<Customer>& customer, const uint16_t current_time)
{
    m_current_customer = customer;
    m_start_serving_at = current_time;
}

bool
BankSimulator::Employee::busy(void)
const noexcept
{
    return m_current_customer != nullptr;
}

std::shared_ptr<BankSimulator::Customer>
BankSimulator::Employee::done(const uint16_t serve_time, const uint16_t current_time)
noexcept
{
    if (busy() && current_time - m_start_serving_at >= serve_time)
    {
        return std::move(m_current_customer);
    }
    
    return nullptr;
}
#pragma endregion // BankSimulator::Employee

#pragma region BankSimulator::Service
std::vector<uint16_t> BankSimulator::Service::s_weights;

BankSimulator::Service::Service(const std::string& id,
    const uint16_t weight,
    const uint16_t serve_time,
    const size_t employees_count)
        : c_id(id),
          c_weight(weight),
          c_serve_time(serve_time),
          c_employees(employees_count)
{
    for (auto& employee : c_employees)
    {
        employee = std::make_unique<Employee>();
    }
}

void
BankSimulator::Service::choose_service(std::shared_ptr<Customer>& customer, const uint16_t service_number)
{
    float random = get_prob();
    float acc = 0;
    
    for (auto const& service : Bank::get_layer(service_number))
    {
        acc += (float) service->c_weight/s_weights[service_number];
        if (random <= acc)
        {
            service->enter(std::move(customer));
            return;
        }
    }

    std::cout <<  "\n";
}

void
BankSimulator::Service::enter_customer(uint16_t current_time)
{
    std::shared_ptr<Customer> customer(new Customer(current_time));
    choose_service(customer, 0);
}

void
BankSimulator::Service::print(void)
const noexcept
{
    uint16_t not_busy_count = std::count_if(
        c_employees.begin(),
        c_employees.end(),
        [](const std::unique_ptr<Employee>& employee) -> bool
        {
            return !employee->busy();
        });

    // !
    std::cout
        << std::setfill('_')
        << "{" << c_id << " "
            << "[" << std::setw(2) << m_queue.size() << "]"
            << "[" << std::setw(2) << not_busy_count << "]"
        << "} ";
}

void
BankSimulator::Service::enter(const std::shared_ptr<Customer>& customer)
{
    m_queue.push(customer);
}

void
BankSimulator::Service::serve_from_queue(const uint16_t current_time)
{
    for (auto& employee : c_employees)
    {
        if (!employee->busy() && !m_queue.empty())
        {
            auto customer = m_queue.front();
            employee->serve(customer, current_time);
            m_queue.pop();
        }
    }
}

void
BankSimulator::Service::checkout_done_customers(const uint16_t current_time, const uint16_t next)
{
    size_t layers_count = s_weights.size();

    for (auto& employee : c_employees)
    {
        if (auto customer = employee->done(c_serve_time, current_time))
        {
            if (next < layers_count)
            {
                choose_service(customer, next);
                break;
            }
            
            customer->exit(current_time);
        }
    }
}
uint16_t
BankSimulator::Service::get_weight(void)
const noexcept
{
    return c_weight;
}

#pragma endregion // BankSimulator::Service

#pragma region BankSimulator::Bank
std::vector<BankSimulator::ServiceLayer> BankSimulator::Bank::s_services_layers;
uint16_t BankSimulator::Bank::s_current_time = 0;

void
BankSimulator::Bank::add_layer(Service *service, ...)
{
    va_list args;
    va_start(args, service);
    ServiceLayer layer;

    do
    {
        layer.push_back(std::unique_ptr<Service>(service));
    }
    while((service = va_arg(args, Service*)) != nullptr);

    Service::s_weights.push_back(
        std::accumulate(
            layer.begin(),
            layer.end(),
            uint16_t(0),
            [](uint16_t &acc, std::unique_ptr<Service> &layer) -> uint16_t
            {
                return acc + layer->get_weight();
            }));

    std::sort(
        layer.begin(),
        layer.end(),
        [](const std::unique_ptr<Service> &a, const std::unique_ptr<Service> &b) -> bool
        {
            return a->get_weight() < b->get_weight();
        });

    s_services_layers.push_back(std::move(layer));
}

void
BankSimulator::Bank::loop_all(const uint16_t current_time)
{
    uint16_t index = 0;
    for (auto &layer : s_services_layers)
    {
        index++;
        for (auto& service : layer)
        {
            service->checkout_done_customers(current_time, index);
            service->serve_from_queue(current_time);
        }
    }

    print_all();
}

void
BankSimulator::Bank::print_all(void)
noexcept
{
    for (auto const& list : s_services_layers)
    {
        for (auto const& service : list)
        {
            service->print();
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

BankSimulator::ServiceLayer const&
BankSimulator::Bank::get_layer(const size_t index)
{
    return s_services_layers[index];
}

void
BankSimulator::Bank::start_working(const uint16_t working_time)
{
    add_layer(new Service("A", 4, 3, 2), nullptr);
    add_layer(new Service("B", 4, 10, 2), new Service("C", 6, 15, 3), nullptr);

    while (++s_current_time < working_time)
    {
        if (get_prob() < get_prob())
        {
            Service::enter_customer(s_current_time);
        }

        loop_all(s_current_time);
    }
}
#pragma endregion // BankSimulator::Bank