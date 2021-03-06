include_directories(.)

foreach (LINK_DIR ${V8_LINK_DIRECTORIES})
  link_directories("${LINK_DIR}")
endforeach()

################################################################################
## IResearch
################################################################################

set(IRESEARCH_TESTS_SOURCES
  Cluster/ClusterInfo-test.cpp
  IResearch/common.cpp
  IResearch/AgencyMock.cpp
  IResearch/AgencyCommManagerMock.cpp
  IResearch/ClusterCommMock.cpp IResearch/ClusterCommMock.h
  IResearch/Containers-test.cpp
  IResearch/ExpressionFilter-test.cpp
  IResearch/ExpressionContextMock.cpp
  IResearch/IResearchAnalyzerFeature-test.cpp
  IResearch/IResearchComparer-test.cpp
  IResearch/IResearchFeature-test.cpp
  IResearch/IResearchOrder-test.cpp
  IResearch/IResearchView-test.cpp
  IResearch/IResearchViewSort-test.cpp
  IResearch/IResearchViewCoordinator-test.cpp
  IResearch/IResearchViewDBServer-test.cpp
  IResearch/IResearchViewMeta-test.cpp
  IResearch/IResearchDocument-test.cpp
  IResearch/IResearchFilter-test.cpp
  IResearch/IResearchFilterFunction-test.cpp
  IResearch/IResearchFilterBoolean-test.cpp
  IResearch/IResearchFilterCompare-test.cpp
  IResearch/IResearchFilterIn-test.cpp
  IResearch/IResearchIndex-test.cpp
  IResearch/IResearchLink-test.cpp
  IResearch/IResearchLinkCoordinator-test.cpp
  IResearch/IResearchLinkHelper-test.cpp
  IResearch/IResearchLinkMeta-test.cpp
  IResearch/IResearchQuery-test.cpp
  IResearch/IResearchQueryAnd-test.cpp
  IResearch/IResearchQueryBooleanTerm-test.cpp
  IResearch/IResearchQueryComplexBoolean-test.cpp
  IResearch/IResearchQueryExists-test.cpp
  IResearch/IResearchQueryIn-test.cpp
  IResearch/IResearchQueryNullTerm-test.cpp
  IResearch/IResearchQueryNumericTerm-test.cpp
  IResearch/IResearchQueryOr-test.cpp
  IResearch/IResearchQueryOptimization-test.cpp
  IResearch/IResearchQueryPhrase-test.cpp
  IResearch/IResearchQueryScorer-test.cpp
  IResearch/IResearchQuerySelectAll-test.cpp
  IResearch/IResearchQueryStartsWith-test.cpp
  IResearch/IResearchQueryInRange-test.cpp
  IResearch/IResearchQueryStringTerm-test.cpp
  IResearch/IResearchQueryTokens-test.cpp
  IResearch/IResearchQueryValue-test.cpp
  IResearch/IResearchQueryJoin-test.cpp
  IResearch/IResearchQueryAggregate-test.cpp
  IResearch/IResearchQueryTraversal-test.cpp
  IResearch/IResearchQueryOptions-test.cpp
  IResearch/IResearchViewSorted-test.cpp
  IResearch/RestHandlerMock.cpp
  IResearch/IResearchViewNode-test.cpp
  IResearch/VelocyPackHelper-test.cpp
  Aql/WaitingExecutionBlockMock.cpp
  RestHandler/RestAnalyzerHandler-test.cpp
  RestHandler/RestUsersHandler-test.cpp
  RestHandler/RestViewHandler-test.cpp
  RestServer/FlushFeature-test.cpp
  Utils/CollectionNameResolver-test.cpp
  V8Server/v8-analyzers-test.cpp
  V8Server/v8-users-test.cpp
  V8Server/v8-views-test.cpp
  VocBase/LogicalDataSource-test.cpp
  VocBase/LogicalView-test.cpp
  VocBase/vocbase-test.cpp
)

set(ARANGODB_TESTS_SOURCES
  Agency/ActiveFailoverTest.cpp
  Agency/AddFollowerTest.cpp
  Agency/CleanOutServerTest.cpp
  Agency/CleanUpLostCollectionTest.cpp
  Agency/FailedFollowerTest.cpp
  Agency/FailedLeaderTest.cpp
  Agency/FailedServerTest.cpp
  Agency/MoveShardTest.cpp
  Agency/RemoveFollowerTest.cpp
  Agency/StoreTest.cpp
  Agency/SupervisionTest.cpp
  Aql/AllRowsFetcherTest.cpp
  Aql/AqlItemBlockHelper.cpp
  Aql/AqlItemRowTest.cpp
  Aql/CalculationExecutorTest.cpp
  Aql/CountCollectExecutorTest.cpp
  Aql/DateFunctionsTest.cpp
  Aql/DependencyProxyMock.cpp
  Aql/DistinctCollectExecutorTest.cpp
  Aql/EngineInfoContainerCoordinatorTest.cpp
  Aql/EnumerateCollectionExecutorTest.cpp
  Aql/EnumerateListExecutorTest.cpp
  Aql/ExecutionBlockImplTest.cpp
  Aql/ExecutionBlockImplTestInstances.cpp
  Aql/FilterExecutorTest.cpp
  Aql/HashedCollectExecutorTest.cpp
  Aql/LimitExecutorTest.cpp
  Aql/MultiDependencySingleRowFetcherTest.cpp
  Aql/IdExecutorTest.cpp
  Aql/NoResultsExecutorTest.cpp
  Aql/RestAqlHandlerTest.cpp
  Aql/ReturnExecutorTest.cpp
  Aql/RowFetcherHelper.cpp
  Aql/ShortestPathExecutorTest.cpp
  Aql/SingleRowFetcherTest.cpp
  Aql/SortExecutorTest.cpp
  Aql/SortedCollectExecutorTest.cpp
  Aql/SortLimit-test.cpp
  Aql/TestEmptyExecutorHelper.cpp
  Aql/TestExecutorHelper.cpp
  Aql/TraversalExecutorTest.cpp
  Aql/VelocyPackHelper.cpp
  Auth/UserManagerTest.cpp
  Basics/ApplicationServerTest.cpp
  Basics/icu-helper.cpp
  Basics/AttributeNameParserTest.cpp
  Basics/associative-multi-pointer-test.cpp
  Basics/associative-multi-pointer-nohashcache-test.cpp
  Basics/datetime.cpp
  Basics/conversions-test.cpp
  Basics/csv-test.cpp
  Basics/files-test.cpp
  Basics/fpconv-test.cpp
  Basics/json-test.cpp
  Basics/hashes-test.cpp
  Basics/skiplist-test.cpp
  Basics/priorityqueue-test.cpp
  Basics/string-buffer-test.cpp
  Basics/string-utf8-normalize-test.cpp
  Basics/string-utf8-test.cpp
  Basics/string-test.cpp
  Basics/vector-test.cpp
  Basics/structure-size-test.cpp
  Basics/CompileTimeStrlenTest.cpp
  Basics/EndpointTest.cpp
  Basics/HashSetTest.cpp
  Basics/LoggerTest.cpp
  Basics/StringBufferTest.cpp
  Basics/StringUtilsTest.cpp
  Basics/VelocyPackHelper-test.cpp
  Cache/BucketState.cpp
  Cache/CachedValue.cpp
  Cache/FrequencyBuffer.cpp
  Cache/Manager.cpp
  Cache/Metadata.cpp
  Cache/MockScheduler.cpp
  Cache/PlainBucket.cpp
  Cache/PlainCache.cpp
  Cache/Rebalancer.cpp
  Cache/Table.cpp
  Cache/TransactionalBucket.cpp
  Cache/TransactionalCache.cpp
  Cache/TransactionalStore.cpp
  Cache/TransactionManager.cpp
  Cache/TransactionsWithBackingStore.cpp
  Cluster/ClusterCommTest.cpp
  Cluster/ClusterHelpersTest.cpp
  Cluster/ClusterRepairsTest.cpp
  Futures/Future-test.cpp
  Futures/Promise-test.cpp
  Futures/Try-test.cpp
  Geo/GeoConstructorTest.cpp
  Geo/GeoJsonTest.cpp
  Geo/GeoFunctionsTest.cpp
  Geo/NearUtilsTest.cpp
  Geo/ShapeContainerTest.cpp
  Graph/GraphTestTools.cpp
  Graph/ClusterTraverserCacheTest.cpp
  Graph/ConstantWeightShortestPathFinder.cpp
  Graph/KShortestPathsFinder.cpp
  Maintenance/MaintenanceFeatureTest.cpp
  Maintenance/MaintenanceRestHandlerTest.cpp
  Maintenance/MaintenanceTest.cpp
  Mocks/StorageEngineMock.cpp
  Mocks/Servers.cpp
  Pregel/typedbuffer.cpp
  RocksDBEngine/Endian.cpp
  RocksDBEngine/KeyTest.cpp
  RocksDBEngine/IndexEstimatorTest.cpp
  Sharding/ShardDistributionReporterTest.cpp
  SimpleHttpClient/CommunicatorTest.cpp
  Transaction/Context-test.cpp
  Transaction/Manager-test.cpp
  Transaction/RestTransactionHandler-test.cpp
  VocBase/VersionTest.cpp
  ${IRESEARCH_TESTS_SOURCES}
)

if (LINUX)
  # add "-fno-var-tracking" to the compiler flags
  # this speeds up the compilation with optimizations and newer g++ versions
  # and prevents the "variable tracking size limit exceeded warnings" from
  # occurring
  if (CMAKE_COMPILER_IS_GNUCC)
    # also turn off super-expensive global common subexpression elimination for tests
    set_source_files_properties(${ARANGODB_TESTS_SOURCES} PROPERTIES COMPILE_FLAGS "-fno-var-tracking -fno-gcse")
  else ()
    set_source_files_properties(${ARANGODB_TESTS_SOURCES} PROPERTIES COMPILE_FLAGS -fno-var-tracking)
  endif ()
endif ()

add_executable(
  arangodbtests
  ${ARANGODB_TESTS_SOURCES}
  main.cpp
)

#required by fakeit dtor mock -- https://github.com/eranpeer/FakeIt/issues/84
if(CMAKE_CXX_COMPILER_ID STREQUAL GNU)
    target_compile_options(arangodbtests PRIVATE -fno-devirtualize)
endif()

SET_TARGET_PROPERTIES(arangodbtests PROPERTIES COMPILE_FLAGS "-DCATCH_CONFIG_FAST_COMPILE")

target_link_libraries(
  arangodbtests
  arangoserver
  rocksdb
)

target_include_directories(arangodbtests PRIVATE
  ${INCLUDE_DIRECTORIES}
  ${V8_INCLUDE_DIR}
)

# add these includes as system includes because otherwise
# the compiler will emit warnings for fakeit.hpp
target_include_directories(arangodbtests SYSTEM PRIVATE
  ${CMAKE_SOURCE_DIR}/3rdParty/catch
  ${CMAKE_SOURCE_DIR}/3rdParty/fakeit
)

find_package(OpenSSL REQUIRED)
list(APPEND IRESEARCH_LIB_RESOURCES
  "${LIB_EAY_DEBUG_DLL}"
  "${SSL_EAY_DEBUG_DLL}"
  "${LIB_EAY_RELEASE_DLL}"
  "${SSL_EAY_RELEASE_DLL}"
)

foreach(ELEMENT ${IRESEARCH_LIB_RESOURCES})
  add_custom_command(
    TARGET arangodbtests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "copying library resource:" "${ELEMENT}" " -> " "$<TARGET_FILE_DIR:arangodbtests>"
    COMMAND cp -lf ${ELEMENT} $<TARGET_FILE_DIR:arangodbtests> || ${CMAKE_COMMAND} -E copy ${ELEMENT} $<TARGET_FILE_DIR:arangodbtests>
  )
endforeach()
