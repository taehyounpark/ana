#pragma once

#include "ana/concurrent.h"
#include "ana/input.h"
#include "ana/processor.h"

namespace ana
{

template <typename T>
class sample
{

public:
  using dataset_type = T;
  using reader_type = input::read_t<T>;

public:
  sample(long long max_entries=-1);
  virtual ~sample() = default;

  template <typename... Args>
  void open(const Args&... args);

  void open(std::unique_ptr<T> dataset);

  void scale(double w);

  long long get_entries() const;
  double get_weight() const;

protected:
  long long                       m_max_entries;
  double                          m_scale;
  std::unique_ptr<T>              m_dataset;
  input::partition                m_partition;
  concurrent<reader_type>         m_readers;
  concurrent<processor<reader_type>> m_processors;

};

}

template <typename T>
ana::sample<T>::sample(long long max_entries) :
  m_max_entries(max_entries),
  m_scale(1.0)
{}

template <typename T>
template <typename... Args>
void ana::sample<T>::open(const Args&... args)
{
  m_dataset = std::make_unique<T>(args...);

  // partition data
	m_partition = m_dataset->allocate().truncate(m_max_entries).merge(ana::multithread::concurrency());

  // normalize data
  m_scale /= m_dataset->normalize();

  // open readers & processors
  m_readers.clear();
  m_processors.clear();
  for (unsigned int islot=0 ; islot<m_partition.size() ; ++islot) {
    auto rdr = m_dataset->open_reader(m_partition.part(islot));
    m_readers.add(rdr);
    auto proc = std::make_shared<processor<reader_type>>(*rdr,m_scale);
    m_processors.add(proc);
	}
}

template <typename T>
void ana::sample<T>::open(std::unique_ptr<T> ds)
{
  m_dataset = std::move(ds);

  // partition data
	m_partition = m_dataset->allocate().truncate(m_max_entries).merge(ana::multithread::concurrency());

  // normalize data
  m_scale /= m_dataset->normalize();

  // open readers & processors
  m_readers.clear();
  m_processors.clear();
  for (unsigned int islot=0 ; islot<m_partition.size() ; ++islot) {
    auto rdr = m_dataset->open_reader(m_partition.part(islot));
    m_readers.add(rdr);
    auto proc = std::make_shared<processor<reader_type>>(*rdr,m_scale);
    m_processors.add(proc);
	}
}

template <typename T>
void ana::sample<T>::scale(double s)
{
  m_scale *= s;
}

template <typename T>
long long ana::sample<T>::get_entries() const
{
  return m_partition.total().entries();
}

template <typename T>
double ana::sample<T>::get_weight() const
{
  return m_scale;
}
