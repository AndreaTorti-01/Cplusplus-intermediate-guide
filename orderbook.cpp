/*
 * Basic Limit Order Matching Engine
 *
 * Features:
 * - Price-time priority matching: Orders matched first by best price, then by arrival time (FIFO)
 * - Buy orders sorted descending (highest price first), sell orders ascending (lowest price first)
 * - Automatic matching on order insertion when bid price >= ask price
 * - Partial fills supported: Orders can be partially filled across multiple matches
 * - Order deduplication: Duplicate order IDs are rejected
 * - Trade execution: Returns all trades generated from a single order insertion
 * - Efficient lookups: O(1) order cancellation capability via hash map
 *
 * Matching Logic:
 * - Continuous matching: After adding an order, matches repeatedly until no cross exists
 * - Match price: Uses the price of the resting order (market maker gets their price)
 * - Order removal: Fully filled orders automatically removed from book
 *
 * Print functions and the tests (main function) are AI-generated.
 */

#include <iostream>
#include <map>
#include <list>
#include <unordered_map>
#include <memory>
#include <cstdint>
#include <vector>
#include <iomanip>
#include <cassert>

using Price = std::int32_t;
using Quantity = std::uint32_t;
using OrderId = std::uint64_t;

enum class Side
{
    Buy,
    Sell
};

class Order
{
private:
    OrderId id_;
    Side side_;
    Price price_;
    Quantity quantity_;

public:
    Order(OrderId id, Side side, Price price, Quantity quantity) : id_(id), side_(side), price_(price),
                                                                   quantity_(quantity)
    {
    }

    OrderId getId() { return id_; }
    Side getSide() { return side_; }
    Price getPrice() { return price_; }
    Quantity getQuantity() { return quantity_; }

    void Fill(const Quantity filling)
    {
        if (filling > quantity_)
        {
            throw std::logic_error("filling too much");
        }
        quantity_ -= filling;
    }

    bool isFilled() const
    {
        return quantity_ == 0;
    }

    void print() const
    {
        using namespace std;
        cout << "Id: " << id_ << " Side: " << static_cast<int>(side_) << " Price: " << price_ << " Quantity: " << quantity_ << '\n';
    }
};

using OrderPointer = std::shared_ptr<Order>;

using OrderPointers = std::list<OrderPointer>;

struct TradeSide
{
    OrderId orderId;
    Price price;
    Quantity quantity;
};

struct Trade
{
    TradeSide buySide;
    TradeSide sellSide;

    void print() const
    {
        using namespace std;
        cout << "Buy: " << buySide.orderId << ' ' << buySide.price << ' ' << buySide.quantity << '\n';
        cout << "Sell: " << sellSide.orderId << ' ' << sellSide.price << ' ' << sellSide.quantity << '\n';
    }
};

using Trades = std::vector<Trade>;

class Orderbook
{
    struct OrderEntry
    {
        OrderPointer order_;
        OrderPointers::iterator iter_;
    };

    std::map<Price, OrderPointers, std::greater<>> bids_;
    std::map<Price, OrderPointers, std::less<>> asks_;
    std::unordered_map<OrderId, OrderEntry> orders_hashmap;

    Trades MatchOrders()
    {
        Trades trades = {};

        while (!asks_.empty() && !bids_.empty())
        {
            auto &[bestBidPrice, bestBids] = *bids_.begin();
            auto &[bestAskPrice, bestAsks] = *asks_.begin();
            if (bestBidPrice < bestAskPrice)
            {
                return trades;
            }

            while (!bestBids.empty() && !bestAsks.empty())
            {
                const auto &oldestBid = bestBids.front();
                const auto &oldestAsk = bestAsks.front();
                const Quantity &bidQty = oldestBid->getQuantity();
                const Quantity &askQty = oldestAsk->getQuantity();
                const Quantity match_qty = std::min(bidQty, askQty);
                const OrderId buyId = oldestBid->getId();
                const OrderId sellId = oldestAsk->getId();
                oldestBid->Fill(match_qty);
                oldestAsk->Fill(match_qty);

                TradeSide buySide{buyId, oldestBid->getPrice(), match_qty};
                TradeSide sellSide{sellId, oldestAsk->getPrice(), match_qty};
                trades.push_back({buySide, sellSide});

                if (oldestBid->isFilled())
                {
                    bestBids.pop_front();
                    orders_hashmap.erase(buyId);
                }
                if (oldestAsk->isFilled())
                {
                    bestAsks.pop_front();
                    orders_hashmap.erase(sellId);
                }
            }
            if (bestAsks.empty())
            {
                asks_.erase(asks_.begin()); // bestAsks invalid now
            }
            if (bestBids.empty())
            {
                bids_.erase(bids_.begin()); // bestBids invalid now
            }
        }

        return trades;
    }

public:
    Trades AddOrder(OrderPointer new_order)
    {
        if (orders_hashmap.contains(new_order->getId()))
        {
            return {};
        }

        OrderPointers::iterator it;
        if (new_order->getSide() == Side::Buy)
        {
            auto &bids_at_price = bids_[new_order->getPrice()];
            it = bids_at_price.insert(bids_at_price.end(), new_order);
        }
        else
        {
            auto &asks_at_price = asks_[new_order->getPrice()];
            it = asks_at_price.insert(asks_at_price.end(), new_order);
        }
        orders_hashmap[new_order->getId()] = {new_order, it};

        return MatchOrders();
    }

    int Size() const
    {
        return orders_hashmap.size();
    }

    // Friend function for printing
    friend void print(const Orderbook &ob);
};

// Overload 1: Print the current orderbook state
void print(const Orderbook &ob)
{
    using namespace std;

    cout << "\n"
         << string(60, '=') << "\n";
    cout << "ORDERBOOK STATUS (Total Orders: " << ob.Size() << ")\n";
    cout << string(60, '=') << "\n\n";

    // Print Asks (Sell orders) - reverse iterate to show highest first
    cout << "ASKS (Sell Orders):\n";
    cout << string(60, '-') << "\n";
    if (ob.asks_.empty())
    {
        cout << "  (empty)\n";
    }
    else
    {
        cout << setw(10) << "Price" << " | " << setw(10) << "Quantity" << " | " << "Order IDs\n";
        cout << string(60, '-') << "\n";

        for (auto it = ob.asks_.rbegin(); it != ob.asks_.rend(); ++it)
        {
            const auto &[price, orders] = *it;
            Quantity totalQty = 0;
            vector<OrderId> ids;

            for (const auto &order : orders)
            {
                totalQty += order->getQuantity();
                ids.push_back(order->getId());
            }

            cout << setw(10) << price << " | " << setw(10) << totalQty << " | ";
            for (size_t i = 0; i < ids.size(); ++i)
            {
                if (i > 0)
                    cout << ", ";
                cout << ids[i];
            }
            cout << "\n";
        }
    }

    cout << "\n"
         << string(60, '-') << "\n";
    cout << "                      SPREAD\n";
    cout << string(60, '-') << "\n\n";

    // Print Bids (Buy orders)
    cout << "BIDS (Buy Orders):\n";
    cout << string(60, '-') << "\n";
    if (ob.bids_.empty())
    {
        cout << "  (empty)\n";
    }
    else
    {
        cout << setw(10) << "Price" << " | " << setw(10) << "Quantity" << " | " << "Order IDs\n";
        cout << string(60, '-') << "\n";

        for (const auto &[price, orders] : ob.bids_)
        {
            Quantity totalQty = 0;
            vector<OrderId> ids;

            for (const auto &order : orders)
            {
                totalQty += order->getQuantity();
                ids.push_back(order->getId());
            }

            cout << setw(10) << price << " | " << setw(10) << totalQty << " | ";
            for (size_t i = 0; i < ids.size(); ++i)
            {
                if (i > 0)
                    cout << ", ";
                cout << ids[i];
            }
            cout << "\n";
        }
    }

    cout << string(60, '=') << "\n\n";
}

// Overload 2: Print trades
void print(const Trades &trades)
{
    using namespace std;

    if (trades.empty())
    {
        cout << "\n[No trades executed]\n\n";
        return;
    }

    cout << "\n"
         << string(60, '=') << "\n";
    cout << "TRADES EXECUTED (" << trades.size() << " trade" << (trades.size() > 1 ? "s" : "") << ")\n";
    cout << string(60, '=') << "\n\n";

    for (size_t i = 0; i < trades.size(); ++i)
    {
        const auto &trade = trades[i];
        cout << "Trade #" << (i + 1) << ":\n";
        cout << "  Buy Side:  OrderID " << setw(6) << trade.buySide.orderId
             << " | Price: " << setw(6) << trade.buySide.price
             << " | Qty: " << setw(6) << trade.buySide.quantity << "\n";
        cout << "  Sell Side: OrderID " << setw(6) << trade.sellSide.orderId
             << " | Price: " << setw(6) << trade.sellSide.price
             << " | Qty: " << setw(6) << trade.sellSide.quantity << "\n";
        cout << string(60, '-') << "\n";
    }
    cout << "\n";
}

int main()
{
    using namespace std;

    cout << "\n*** LIMIT ORDER MATCHING ENGINE TEST ***\n";

    Orderbook ob;
    Trades test_trades;

    // Test 1: Empty orderbook
    cout << "\n=== TEST 1: Initial State (Empty Orderbook) ===";
    print(ob);
    assert(ob.Size() == 0 && "Empty orderbook should have size 0");

    // Test 2: Add buy order with no match
    cout << "\n=== TEST 2: Add Buy Order (ID=1, Price=100, Qty=10) ===";
    test_trades = ob.AddOrder(make_shared<Order>(1, Side::Buy, 100, 10));
    print(test_trades);
    print(ob);
    assert(test_trades.size() == 0 && "No matching orders, should produce 0 trades");
    assert(ob.Size() == 1 && "One order in book");

    // Test 3: Add sell order above best bid (no match)
    cout << "\n=== TEST 3: Add Sell Order (ID=2, Price=105, Qty=5) ===";
    test_trades = ob.AddOrder(make_shared<Order>(2, Side::Sell, 105, 5));
    print(test_trades);
    print(ob);
    assert(test_trades.size() == 0 && "Sell price > buy price, no match");
    assert(ob.Size() == 2 && "Two orders in book");

    // Test 4: Add another buy order at different price
    cout << "\n=== TEST 4: Add Buy Order (ID=3, Price=98, Qty=8) ===";
    test_trades = ob.AddOrder(make_shared<Order>(3, Side::Buy, 98, 8));
    print(test_trades);
    print(ob);
    assert(test_trades.size() == 0 && "No match at this price");
    assert(ob.Size() == 3 && "Three orders in book");

    // Test 5: Exact match
    cout << "\n=== TEST 5: Add Sell Order Matching Best Bid (ID=4, Price=100, Qty=10) ===";
    test_trades = ob.AddOrder(make_shared<Order>(4, Side::Sell, 100, 10));
    print(test_trades);
    print(ob);
    assert(test_trades.size() == 1 && "Should produce exactly 1 trade");
    assert(test_trades[0].buySide.orderId == 1 && "Buy side should be order 1");
    assert(test_trades[0].sellSide.orderId == 4 && "Sell side should be order 4");
    assert(test_trades[0].buySide.quantity == 10 && "Trade quantity should be 10");
    assert(ob.Size() == 2 && "Both matched orders removed, 2 remain");

    // Test 6: Partial fill - sell order larger than buy
    cout << "\n=== TEST 6: Partial Fill - Sell > Buy (ID=5, Price=98, Qty=15) ===";
    test_trades = ob.AddOrder(make_shared<Order>(5, Side::Sell, 98, 15));
    print(test_trades);
    print(ob);
    assert(test_trades.size() == 1 && "One trade from partial fill");
    assert(test_trades[0].buySide.quantity == 8 && "Buy order fully filled with 8");
    assert(test_trades[0].sellSide.quantity == 8 && "Sell order partially filled with 8");
    assert(ob.Size() == 2 && "Buy order removed, partial sell remains");

    // Test 7: Multiple price levels - rebuild book
    cout << "\n=== TEST 7: Build Multi-Level Book ===";
    Trades accumulated_trades;
    auto t1 = ob.AddOrder(make_shared<Order>(6, Side::Buy, 102, 20));
    accumulated_trades.insert(accumulated_trades.end(), t1.begin(), t1.end());
    auto t2 = ob.AddOrder(make_shared<Order>(7, Side::Buy, 101, 15));
    accumulated_trades.insert(accumulated_trades.end(), t2.begin(), t2.end());
    auto t3 = ob.AddOrder(make_shared<Order>(8, Side::Buy, 100, 10));
    accumulated_trades.insert(accumulated_trades.end(), t3.begin(), t3.end());
    auto t4 = ob.AddOrder(make_shared<Order>(9, Side::Sell, 108, 12));
    accumulated_trades.insert(accumulated_trades.end(), t4.begin(), t4.end());
    auto t5 = ob.AddOrder(make_shared<Order>(10, Side::Sell, 109, 18));
    accumulated_trades.insert(accumulated_trades.end(), t5.begin(), t5.end());
    print(accumulated_trades);
    print(ob);
    assert(accumulated_trades.size() == 1 && "Order 6 matches remaining sell order 5");
    assert(ob.Size() == 6 && "6 orders total in book (order 5 and 6 partially matched)");

    // Test 8: Aggressive sell order matching multiple levels
    cout << "\n=== TEST 8: Aggressive Sell Crossing Multiple Levels (ID=11, Price=100, Qty=40) ===";
    test_trades = ob.AddOrder(make_shared<Order>(11, Side::Sell, 100, 40));
    print(test_trades);
    print(ob);
    assert(test_trades.size() == 3 && "Should match 3 buy orders at different levels");
    assert(test_trades[0].buySide.orderId == 6 && "First match with order 6 (price 102)");
    assert(test_trades[1].buySide.orderId == 7 && "Second match with order 7 (price 101)");
    assert(test_trades[2].buySide.orderId == 8 && "Third match with order 8 (price 100)");
    assert(ob.Size() == 4 && "3 buy orders removed, 1 partial sell (ID 11) remains");

    // Test 9: Multiple orders at same price (FIFO test)
    // NOTE: Changed Price to 90. Best Ask is currently 100 (Order 11).
    // To rest in the book, the Buy Price must be < Best Ask.
    cout << "\n=== TEST 9: FIFO Test - Multiple Orders at Same Price (90) ===";
    accumulated_trades.clear();
    auto t9a = ob.AddOrder(make_shared<Order>(12, Side::Buy, 90, 5));
    accumulated_trades.insert(accumulated_trades.end(), t9a.begin(), t9a.end());
    auto t9b = ob.AddOrder(make_shared<Order>(13, Side::Buy, 90, 3));
    accumulated_trades.insert(accumulated_trades.end(), t9b.begin(), t9b.end());
    auto t9c = ob.AddOrder(make_shared<Order>(14, Side::Buy, 90, 7));
    accumulated_trades.insert(accumulated_trades.end(), t9c.begin(), t9c.end());
    print(accumulated_trades);
    print(ob);
    assert(accumulated_trades.empty() && "Orders should rest, not match");
    assert(ob.Size() == 7 && "7 orders in book (4 previous Asks + 3 new Bids)");

    // Test 9b: FIFO Execution
    // NOTE: Sell Price set to 90 to cross Bids. Qty set to 8 to match exactly ID 12(5) and 13(3).
    cout << "\n=== TEST 9b: Sell Order Matching FIFO Queue (ID=15, Price=90, Qty=8) ===";
    test_trades = ob.AddOrder(make_shared<Order>(15, Side::Sell, 90, 8));
    print(test_trades);
    print(ob);
    assert(test_trades.size() == 2 && "Should match exactly 2 orders (5+3=8)");
    assert(test_trades[0].buySide.orderId == 12 && "First in queue: order 12");
    assert(test_trades[1].buySide.orderId == 13 && "Second in queue: order 13");
    assert(ob.Size() == 5 && "2 Buy orders removed. Remaining: 4 Asks + 1 Bid (Order 14)");

    // Test 10: Duplicate order ID
    // NOTE: Using ID 11 (which exists in book) to verify rejection.
    // (ID 12 was removed in Test 9b, so re-adding it would actually be valid).
    cout << "\n=== TEST 10: Duplicate Order ID (ID=11 again) ===";
    test_trades = ob.AddOrder(make_shared<Order>(11, Side::Sell, 200, 5));
    print(test_trades);
    print(ob);
    cout << "(Should show no trades - duplicate rejected)\n";
    assert(test_trades.size() == 0 && "Duplicate order should be rejected");
    assert(ob.Size() == 5 && "Size unchanged after duplicate rejection");

    // Test 11: Final aggressive order clearing remaining book
    // NOTE: Price 80 is aggressive enough to cross remaining Bid at 90.
    cout << "\n=== TEST 11: Clear Remaining Bids (ID=16, Price=80, Qty=100) ===";
    test_trades = ob.AddOrder(make_shared<Order>(16, Side::Sell, 80, 100));
    print(test_trades);
    print(ob);
    assert(test_trades.size() == 1 && "Should match remaining buy order (ID 14)");
    assert(test_trades[0].buySide.orderId == 14);
    assert(ob.Size() == 5 && "Bid 14 removed, new Sell 16 added (rests with qty 93). Total 5 Asks.");

    cout << "\n*** ALL TESTS COMPLETED SUCCESSFULLY ***\n\n";

    return 0;
}