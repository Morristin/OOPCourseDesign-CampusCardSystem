#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <functional>
#include <initializer_list>
#include <string>

class Dashboard;

struct MenuItem {
    std::string name;
    std::string description;
    std::variant<std::function<void()>, Dashboard*> value;
};

class Dashboard {
private:
    std::vector<MenuItem> menu_;
    Dashboard* previous_dashboard_ = nullptr;

public:
    Dashboard(const std::initializer_list<MenuItem> menu_items)
    {
        for (const auto& menu_item : menu_items) {
            if (std::holds_alternative<Dashboard*>(menu_item.value)) {
                const auto sub_dashboard = std::get<Dashboard*>(menu_item.value);
                sub_dashboard->previous_dashboard_ = this;
            }
            menu_.push_back(menu_item);
        }
    }

    [[nodiscard]] Dashboard& previous_dashboard() const { return *previous_dashboard_; }
    [[nodiscard]] unsigned long length() const { return menu_.size(); }
    MenuItem operator[](const int index) const { return menu_[index - 1]; }
};

#endif
