#ifndef CASM_unit_Common
#define CASM_unit_Common

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "casm/core"

using namespace CASM;

namespace test {

  inline void check(std::string test,
                    const jsonParser &expected,
                    const jsonParser &calculated,
                    fs::path test_cases_path,
                    bool quiet) {
    if(expected.contains(test)) {
      BOOST_CHECK_EQUAL(expected[test], calculated);
    }
    else if(!quiet) {
      std::cout << "Test case: " << expected["title"] << " has no \"" << test << "\" test data." << std::endl;
      std::cout << "To use the current CASM results, add the following to the " << expected["title"]
                << " test case in " << test_cases_path << std::endl;
      jsonParser j = jsonParser::object();
      j[test] = calculated;
      std::cout << j << std::endl;
    }
  }

  class Proj {

  public:

    Proj(fs::path _proj_dir,
         const BasicStructure<Site> &_prim,
         std::string _title,
         std::string _desc) :
      dir(_proj_dir),
      prim(_prim),
      title(_title),
      desc(_desc),
      m_dirs(dir) {}

    fs::path dir;
    BasicStructure<Site> prim;
    std::string title;
    std::string desc;

    std::string cd_and() const {
      return "cd " + dir.string() + "&& ";
    };

    /// \brief Check project initialization
    virtual void check_init();

    /// \brief Check symmetry
    virtual void check_symmetry();

    /// \brief Check "casm composition"
    virtual void check_composition();

    /// \brief Check "casm bset"
    virtual void check_bset() = 0;

    /// \brief Check "casm enum"
    virtual void check_enum();

    /// \brief Check "casm select"
    virtual void check_select();

    /// \brief Check "casm query"
    virtual void check_query();


  protected:

    Popen m_p;
    boost::smatch m_match;
    DirectoryStructure m_dirs;
    ProjectSettings m_set;

    void _check_symmetry(int lat_pg_op, int lat_pg_class,
                         int xtal_pg_op, int xtal_pg_class,
                         int fg_op, int fg_class,
                         std::string lat_pg_name, std::string xtal_pg_name);

    template<typename Iterator>
    void _check_composition_axes(Iterator begin, Iterator end);

  };

  /// \brief Build a CASM project at 'proj_dir' using the prim
  ///
  /// Typically, proj_dir.filename() == title
  void make_project(const Proj &proj);

  /// \brief Remove a CASM project, checking first that there is a '.casm' dir
  ///
  /// Be careful! This does a recursive remove of the entire proj_dir!
  void rm_project(const Proj &proj);

  /// \brief Check some aspects of a SymGroup json, including the expected
  ///        number of conjugacy classes and operations
  void check_symgroup(const jsonParser &json, int N_op, int N_class);

}

#include "FCCTernaryProj.hh"
#include "ZrOProj.hh"

namespace test {

  template<typename Iterator>
  void Proj::_check_composition_axes(Iterator begin, Iterator end) {

    m_p.popen(cd_and() + "casm composition --select 0");

    for(auto it = begin; it != end; ++it) {
      BOOST_CHECK_MESSAGE(boost::regex_search(m_p.gets(), m_match, boost::regex(*it)) == true, m_p.gets());
    }

    BOOST_CHECK_MESSAGE(
      boost::regex_search(m_p.gets(), m_match, boost::regex(R"(Currently selected composition axes: 0)")) ==
      true,
      m_p.gets()
    );
  }
}

#endif