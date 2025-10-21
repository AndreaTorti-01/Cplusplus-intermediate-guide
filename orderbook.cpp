#include <cstdint>
#include <vector>
#include <format>
#include <list>
#include <memory>
#include <unordered_map>
#include <map>
#include <functional>
#include <numeric>
#include <iostream>

// class places the inside keywords inside the enum's namespace (OrderType::GoodTillCancel)
enum class OrderType
{
    GoodTillCancel, // 0
    FillAndKill     // 1
};

enum class Side
{
    Buy,
    Sell
};

using Price = std::int32_t;
using Quantity = std::uint32_t;
using OrderID = std::uint64_t;

struct LevelInfo
{
    Price price_;
    Quantity quantity_;
};

using LevelInfos = std::vector<LevelInfo>;

class OrderbookLevelInfos
{
public:
    OrderbookLevelInfos(const LevelInfos &bids, const LevelInfos &asks)
        : bids_{bids}, asks_{asks} {}

    const LevelInfos &getBids() const noexcept { return bids_; }
    const LevelInfos &getAsks() const noexcept { return asks_; }

private:
    LevelInfos bids_, asks_;
};

class Order
{
public:
    Order(OrderType orderType, OrderID orderID, Side side, Price price, Quantity quantity)
        : orderType_{orderType}, orderID_{orderID}, side_{side}, price_{price}, initialQuantity_{quantity}, remainingQuantity_{quantity} {};

    OrderType getOrderType() const { return orderType_; }
    OrderID getOrderID() const { return orderID_; }
    Side getSide() const { return side_; }
    Price getPrice() const { return price_; }
    Quantity getInitialQuantity() const { return initialQuantity_; }
    Quantity getRemainingQuantity() const { return remainingQuantity_; }
    Quantity getFilledQuantity() const { return initialQuantity_ - remainingQuantity_; }
    bool isFilled() const { return remainingQuantity_ == 0; }
    bool isBuy() const { return side_ == Side::Buy; }
    bool isSell() const { return side_ == Side::Sell; }

    void Fill(Quantity quantity)
    {
        if (quantity > getRemainingQuantity())
        {
            throw std::logic_error(std::format("Order ({}) could not be filled for more than its remaining quantity.", getOrderID()));
        }

        remainingQuantity_ -= quantity;
    }

private:
    OrderType orderType_;
    OrderID orderID_;
    Side side_;
    Price price_;
    Quantity initialQuantity_, remainingQuantity_;
};

using OrderPointer = std::shared_ptr<Order>;
// TODO why not a forward_list or a vector
using OrderPointers = std::list<OrderPointer>;

class OrderModify
{
public:
    OrderModify(OrderID orderID, Side side, Price price, Quantity quantity)
        : orderID_{orderID}, side_{side}, price_{price}, newQuantity_{quantity} {};

    OrderID getOrderID() const { return orderID_; }
    Price getPrice() const { return price_; }
    Side getSide() const { return side_; }
    Quantity getQuantity() const { return newQuantity_; }

    OrderPointer ToOrderPointer(OrderType type) const
    {
        return std::make_shared<Order>(type, getOrderID(), getSide(), getPrice(), getQuantity());
    }

private:
    OrderID orderID_;
    Side side_;
    Price price_;
    Quantity newQuantity_;
};

// trade info, a trade object needs two...
struct TradeInfo
{
    OrderID orderID_;
    Price price_;
    Quantity quantity_;
};

// ...the ask side and the bid side
class Trade
{
public:
    Trade(const TradeInfo &bidTrade, const TradeInfo &askTrade)
        : bidTrade_{bidTrade}, askTrade_{askTrade} {};

    const TradeInfo &getBidTrade() const { return bidTrade_; }
    const TradeInfo &getAskTrade() const { return askTrade_; }

private:
    TradeInfo bidTrade_, askTrade_;
};

using Trades = std::vector<Trade>;

class OrderBook
{
private:
    struct OrderEntry
    {
        // a pointer to the actual order
        OrderPointer order_{nullptr};
        // iterator to the list of order pointers
        OrderPointers::iterator location_;
    };

    std::map<Price, OrderPointers, std::greater<Price>> bids_;
    std::map<Price, OrderPointers, std::less<Price>> asks_;
    std::unordered_map<OrderID, OrderEntry> orders_;

    bool CanMatch(Side side, Price price) const
    {
        if (side == Side::Buy)
        {
            if (asks_.empty())
                return false;

            const auto &[bestAsk, _] = *asks_.begin(); // structured binding: bestAsk is the price, _ means I don't care (about the OrderPointers list)

            return price >= bestAsk;
        }

        else // if selling
        {
            if (bids_.empty())
                return false;

            const auto &[bestBid, _] = *bids_.begin(); // structured binding: bestAsk is the price, _ means I don't care (about the OrderPointers list)

            return price <= bestBid;
        }
    }

    // returns a vector of trade(s)
    Trades MatchOrders()
    {
        Trades trades;                  // empty vector of trades
        trades.reserve(orders_.size()); // as many trades as the orders number...

        while (true) // FOR EACH (matching) price...
        {
            if (bids_.empty() || asks_.empty())
                break; // if no bids or no asks in the book then no trades

            auto &[bidPrice, bids] = *bids_.begin(); // best bid(s)
            auto &[askPrice, asks] = *asks_.begin(); // best ask(s)

            if (bidPrice < askPrice)
                break; // if the best bid is strictly under the best ask then no trades.

            while (bids.size() && asks.size())
            {                            // while there are still bids and asks left
                auto bid = bids.front(); // first of the LIST (first order added in time: FIFO)
                auto ask = asks.front();
                // auto &bid could lead to a serious bug when popping it! copying a shared ptr is very cheap so let's do it.

                // there is a trade! the quantity is the minimum between the two quantities
                Quantity quantity = std::min(bid->getRemainingQuantity(), ask->getRemainingQuantity());

                bid->Fill(quantity); // subtracts
                ask->Fill(quantity);

                // if this check was to be done in the order filling itself, it wouldn't be removed from the book
                if (bid->isFilled())
                {
                    bids.pop_front(); // remove that bid order from the queue at that price if it is empty
                    orders_.erase(bid->getOrderID());
                    if (bids.empty()) // if there are no more bids at the considered price...
                    {
                        bids_.erase(bidPrice); // remove the current price from the book
                    }
                }

                if (ask->isFilled())
                {
                    asks.pop_front(); // remove that ask order from the queue at that price if it is empty
                    orders_.erase(ask->getOrderID());
                    if (asks.empty()) // if there are no more asks at the considered price...
                    {
                        asks_.erase(askPrice); // remove the current price from the book
                    }
                }

                // TODO price and quantity is obviously the same
                trades.push_back(Trade{
                    TradeInfo{bid->getOrderID(), bid->getPrice(), quantity},
                    TradeInfo{ask->getOrderID(), ask->getPrice(), quantity}});
            }
        }

        if (!bids_.empty())
        {
            auto &[_, bids] = *bids_.begin(); // take the highest bids
            auto &order = bids.front();       // take the oldest order
            if (order->getOrderType() == OrderType::FillAndKill)
            { // if fill and kill delete it
                CancelOrder(order->getOrderID());
            }
        }

        return trades;
    }

    // orderbook class public API
public:
    Trades AddOrder(OrderPointer order)
    {
        if (orders_.contains(order->getOrderID()))
        {
            return {}; // if order id already exists reject order
        }

        if (order->getOrderType() == OrderType::FillAndKill && !CanMatch(order->getSide(), order->getPrice()))
        {
            return {}; // do not add order if fill and kill and no match
        }

        OrderPointers::iterator iterator; // iterator to list of order shared pointers

        if (order->getSide() == Side::Buy)
        {
            auto &orders = bids_[order->getPrice()];                 // take the bids tree node with the correct price or create it
            orders.push_back(order);                                 // add it to the list of orders in that price node
            iterator = std::next(orders.begin(), orders.size() - 1); // set the iterator to the last (newly added) element
        }

        else
        {
            auto &orders = asks_[order->getPrice()];
            orders.push_back(order);
            iterator = std::next(orders.begin(), orders.size() - 1);
        }

        orders_.insert({order->getOrderID(), OrderEntry{order, iterator}}); // add it to the hashmap (which searches order by id and gives back an entry, which is a pointer + iterator to the list of orders)
        return MatchOrders();
    }

    void CancelOrder(OrderID orderID)
    {
        // if it doesn't exist
        if (!orders_.contains(orderID))
            return;

        const auto [order, orderIterator] = orders_.at(orderID); // O(1) access, great!
        orders_.erase(orderID);                                   // remove it from the hashmap

        // now look for it in the right tree to remove it from the list at the x price.
        // we saved the iterator to the list in the hasmap! and also a copy of all the order information, following its pointer.
        // note: if the pointer was missing, to have the info would take a traversal of the tree till we find the price, the
        // price itself (which we don't have) and then nothing else because we would have the iterator. instead, having the information
        // ready is useful even tho we'll reach the same info anyway looking for that iterator in the tree.
        if (order->getSide() == Side::Sell)
        {
            auto price = order->getPrice();
            auto &orders = asks_.at(price); // get the tree node for the order price we want to cancel
            orders.erase(orderIterator);    // delete it
            if (orders.empty())             // if there are no more orders at that price...
                asks_.erase(price);         // ... remove the price from the tree
        }
        else
        {
            auto price = order->getPrice();
            auto &orders = bids_.at(price); // get the tree node for the order price we want to cancel
            orders.erase(orderIterator);    // delete it
            if (orders.empty())             // if there are no more orders at that price...
                bids_.erase(price);         // ... remove the price from the tree
        }
    }

    // Modify is just cancel + add (can generate trades)
    Trades MatchOrder(OrderModify order)
    {
        // return empty vector of trade(s) if there is no order with the same id
        if (!orders_.contains(order.getOrderID()))
            return {};

        // get the pointer to the order (and no iterator, not interested) from the hashmap
        const auto &[existingOrder, _] = orders_.at(order.getOrderID());

        // maybe could also do existingOrder->getOrderID() since they have the same ID
        CancelOrder(order.getOrderID());
        return AddOrder(order.ToOrderPointer(existingOrder->getOrderType()));
    }

    std::size_t Size() const { return orders_.size(); }

    OrderbookLevelInfos GetOrderInfos() const
    {
        LevelInfos bidInfos, askInfos;
        bidInfos.reserve(orders_.size()); // worst case. could not ask the tree because it contains lists :(
        askInfos.reserve(orders_.size());

        auto CreateLevelInfos = [](Price price, const OrderPointers &orders)
        {
            return LevelInfo{price, std::accumulate(orders.begin(), orders.end(), (Quantity)0,
                                                    [](Quantity runningSum, const OrderPointer &order)
                                                    { return runningSum + order->getRemainingQuantity(); })};
        };
        // the inner lambda extracts the running sum of quantities for orders at a given price.
        // the outer one just accumulates, returning the running sum of quantities.
        // then the function returns a LevelInfo structure with price and total quantity

        for (const auto &[price, orders] : bids_)
        {
            bidInfos.push_back(CreateLevelInfos(price, orders));
        }
        for (const auto &[price, orders] : asks_)
        {
            askInfos.push_back(CreateLevelInfos(price, orders));
        }

        return OrderbookLevelInfos{bidInfos, askInfos};
    }
};

// very basic testing.
int main()
{
    OrderBook orderbook;
    const OrderID orderID = 1;
    orderbook.AddOrder(std::make_shared<Order>(OrderType::GoodTillCancel, orderID, Side::Buy, 100, 10));
    std::cout << orderbook.Size() << std::endl;
    orderbook.CancelOrder(orderID);
    std::cout << orderbook.Size() << std::endl;

    return 0;
}