
#include <iostream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <list>
#include <forward_list>
#include <deque>

#include <benchmark/benchmark.h>

using LargeThing = std::array<int, 1024>;

struct Untimed {
  Untimed( benchmark::State &state ) : m_state{state} { m_state.PauseTiming(); }
  ~Untimed() noexcept { m_state.ResumeTiming(); }
  benchmark::State &m_state;
};

static void reportCounters( benchmark::State &state, size_t bytes_per_item ) {
  state.SetItemsProcessed( static_cast<int64_t>( state.iterations() ) * state.range( 0 ) );
  state.SetBytesProcessed( static_cast<int64_t>( state.iterations() ) * state.range( 0 ) * bytes_per_item );
  state.counters["size"] = state.range( 0 );
}

static const std::vector<std::pair<int64_t, int64_t>> g_default_ranges {
  {1 << 0, 10 << 10},
  {10 << 10, 10 << 10}
};

/* ---- Insertion Benchmarks ---- */

template <typename SeqContainer>
void BM_InsertFront( benchmark::State &state ) {
  typename SeqContainer::value_type value;
  for ( auto _ : state ) {
    SeqContainer container;
    std::fill_n( std::inserter( container, container.begin() ), state.range( 0 ), value );
  }
  reportCounters( state, sizeof(value) );
}

template <typename SeqContainer>
void BM_InsertBack( benchmark::State &state ) {
  typename SeqContainer::value_type value;
  for ( auto _ : state ) {
    SeqContainer container;
    std::fill_n( std::back_inserter( container ), state.range( 0 ), value );
  }
  reportCounters( state, sizeof( value ) );
}

template <typename SeqContainer>
void BM_InsertMiddle( benchmark::State &state ) {
  typename SeqContainer::value_type value;
  for ( auto _ : state ) {
    state.PauseTiming();
    SeqContainer container;
    std::fill_n( std::back_inserter( container ), state.range( 1 ), value );
    auto iter = container.begin();
    std::advance( iter, state.range( 1 ) / 2 );
    state.ResumeTiming();
    std::fill_n( std::inserter( container, iter ), state.range( 0 ), value );
  }
  reportCounters( state, sizeof( value ) );
}

/* ---- Removal Benchmarks ---- */

template <typename SeqContainer>
void BM_RemoveFront( benchmark::State &state ) {
  typename SeqContainer::value_type value;
  for ( auto _ : state ) {
    state.PauseTiming();
    SeqContainer container;
    std::fill_n( std::back_inserter( container ), state.range( 0 ), value );
    state.ResumeTiming();
    while ( !container.empty() ) {
      container.erase( container.begin() );
    }
  }
  reportCounters( state, sizeof( value ) );
}

template <typename SeqContainer>
void BM_RemoveBack( benchmark::State &state ) {
  typename SeqContainer::value_type value;
  for ( auto _ : state ) {
    state.PauseTiming();
    SeqContainer container;
    std::fill_n( std::back_inserter( container ), state.range( 0 ), value );
    state.ResumeTiming();
    while ( !container.empty() ) {
      container.pop_back();
    }
  }
  reportCounters( state, sizeof( value ) );
}

template <typename SeqContainer>
void BM_RemoveMiddle( benchmark::State &state ) {
  typename SeqContainer::value_type value;
  if ( state.range( 0 ) > state.range( 1 ) ) {
    state.SkipWithError( "Range to remove is larger than range to initialize" );
  }
  for ( auto _ : state ) {
    state.PauseTiming();
    SeqContainer container;
    std::fill_n( std::back_inserter( container ), state.range( 1 ) + 1, value );
    auto iter = container.begin();
    std::advance( iter, (state.range( 1 ) - state.range( 0 ))/ 2 );
    state.ResumeTiming();
    for ( size_t pos = 0, end = state.range( 0 ); pos < end; pos++ ) {
      iter = container.erase( iter );
    }
  }
  if ( state.range( 1 ) >= state.range( 0 ) ) {
    reportCounters( state, sizeof( value ) );
  }
}

/* ---- Insertion Benchmarks ---- */

template <typename SeqContainer>
void BM_AccessForward( benchmark::State &state ) {
  typename SeqContainer::value_type value;
  SeqContainer container;
  std::fill_n( std::back_inserter( container ), state.range( 0 ), value );
  for ( auto _ : state ) {
    for ( auto &accessed : container ) {
      if constexpr (std::is_same_v<typename SeqContainer::value_type, int>) {
        benchmark::DoNotOptimize( accessed ^ accessed );
      } else {
        benchmark::DoNotOptimize( accessed );
      }
    }
  }
  reportCounters( state, sizeof( value ) );
}

template <typename SeqContainer>
void BM_AccessBackward( benchmark::State &state ) {
  typename SeqContainer::value_type value;
  SeqContainer container;
  std::fill_n( std::back_inserter( container ), state.range( 0 ), value );
  for ( auto _ : state ) {
    for ( auto iter = std::rbegin( container ), end = std::rend( container ); iter != end; iter++ ) {
      benchmark::DoNotOptimize( *iter );
    }
  }
  reportCounters( state, sizeof( value ) );
}

template <typename SeqContainer>
void BM_AccessRandom( benchmark::State &state ) {
  typename SeqContainer::value_type value;
  SeqContainer container;
  std::fill_n( std::back_inserter( container ), state.range( 0 ) + 1, value );
  size_t size = state.range( 0 );
  for ( auto _ : state ) {
    // NOTE this algorithm assumes an even range
    for ( size_t pos = size / 2; pos > 0; pos-- ) {
      auto iter = container.begin();
      std::advance( iter, pos );
      benchmark::DoNotOptimize( *iter );
      iter = container.begin();
      std::advance( iter, size - pos );
      benchmark::DoNotOptimize( *iter );
    }
  }
  reportCounters( state, sizeof( value ) );
}

BENCHMARK_TEMPLATE( BM_InsertFront, std::vector<int> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_InsertFront, std::list<int> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_InsertFront, std::deque<int> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_InsertFront, std::vector<LargeThing> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_InsertFront, std::list<LargeThing> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_InsertFront, std::deque<LargeThing> )->Ranges( g_default_ranges );

BENCHMARK_TEMPLATE( BM_InsertBack, std::vector<int> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_InsertBack, std::list<int> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_InsertBack, std::deque<int> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_InsertBack, std::vector<LargeThing> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_InsertBack, std::list<LargeThing> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_InsertBack, std::deque<LargeThing> )->Ranges( g_default_ranges );

BENCHMARK_TEMPLATE( BM_InsertMiddle, std::vector<int> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_InsertMiddle, std::list<int> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_InsertMiddle, std::deque<int> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_InsertMiddle, std::vector<LargeThing> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_InsertMiddle, std::list<LargeThing> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_InsertMiddle, std::deque<LargeThing> )->Ranges( g_default_ranges );

BENCHMARK_TEMPLATE( BM_RemoveFront, std::vector<int> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_RemoveFront, std::list<int> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_RemoveFront, std::deque<int> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_RemoveFront, std::vector<LargeThing> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_RemoveFront, std::list<LargeThing> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_RemoveFront, std::deque<LargeThing> )->Ranges( g_default_ranges );

BENCHMARK_TEMPLATE( BM_RemoveBack, std::vector<int> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_RemoveBack, std::list<int> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_RemoveBack, std::deque<int> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_RemoveBack, std::vector<LargeThing> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_RemoveBack, std::list<LargeThing> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_RemoveBack, std::deque<LargeThing> )->Ranges( g_default_ranges );

BENCHMARK_TEMPLATE( BM_RemoveMiddle, std::vector<int> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_RemoveMiddle, std::list<int> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_RemoveMiddle, std::deque<int> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_RemoveMiddle, std::vector<LargeThing> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_RemoveMiddle, std::list<LargeThing> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_RemoveMiddle, std::deque<LargeThing> )->Ranges( g_default_ranges );

BENCHMARK_TEMPLATE( BM_AccessForward, std::vector<int> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_AccessForward, std::list<int> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_AccessForward, std::deque<int> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_AccessForward, std::vector<LargeThing> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_AccessForward, std::list<LargeThing> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_AccessForward, std::deque<LargeThing> )->Ranges( g_default_ranges );

BENCHMARK_TEMPLATE( BM_AccessBackward, std::vector<int> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_AccessBackward, std::list<int> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_AccessBackward, std::deque<int> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_AccessBackward, std::vector<LargeThing> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_AccessBackward, std::list<LargeThing> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_AccessBackward, std::deque<LargeThing> )->Ranges( g_default_ranges );

BENCHMARK_TEMPLATE( BM_AccessRandom, std::vector<int> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_AccessRandom, std::list<int> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_AccessRandom, std::deque<int> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_AccessRandom, std::vector<LargeThing> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_AccessRandom, std::list<LargeThing> )->Ranges( g_default_ranges );
BENCHMARK_TEMPLATE( BM_AccessRandom, std::deque<LargeThing> )->Ranges( g_default_ranges );

BENCHMARK_MAIN();
