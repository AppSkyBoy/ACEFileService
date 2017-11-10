using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Formatters.Binary;
using System.Xml;
using System.Xml.Serialization;

namespace ACE_FILE_SERVER_VIEW
{
    /// <summary>
    /// 业务逻辑层帮助类。
    /// </summary>
    public static class XmlHelper
    {
        /// <summary>
        /// Xml序列化对象。
        /// </summary>
        /// <param name="obj">将要序列化的对象。</param>
        /// <returns>返回Xml字符串。</returns>
        public static string SerializeObj(object obj)
        {
            if (obj == null)
                return "";
            var xmlSerializer = new XmlSerializer(obj.GetType());
            var writer = new StringWriter();
            xmlSerializer.Serialize(writer, obj);
            return writer.ToString();
        }
      
        /// <summary>
        /// Xml序列化对象一个空对象。
        /// </summary>
        /// <returns>返回Xml字符串。</returns>
        public static string SerializeObj<T>() where T : class
        {
            var xmlSerializer = new XmlSerializer(typeof(T));
            var writer = new StringWriter();
            xmlSerializer.Serialize(writer, null);
            return writer.ToString();
        }

        /// <summary>
        /// Xml反序列化对象。
        /// </summary>
        /// <typeparam name="T">对象类型。</typeparam>
        /// <param name="xmlStr">Xml字符串。</param>
        /// <returns>返回反序列化后的对象。</returns>
        public static T DeserializeObj<T>(string xmlStr) where T : class
        {
            xmlStr = xmlStr.Replace("  ", "");

            if (string.IsNullOrEmpty(xmlStr))
                return null;
            var serializer = new XmlSerializer(typeof(T));
            using (var reader = new XmlTextReader(new StringReader(xmlStr)))
            {                
                reader.Normalization = false;
                try
                {
                    return (T)serializer.Deserialize(reader);
                }
                catch (Exception ex)
                {                                        
                    return null;
                }
            }

        }
        /// <summary>
        /// 将xml中相应节点下的值赋值给给定的类型实例
        /// </summary>
        /// <typeparam name="T">给定的类型</typeparam>
        /// <param name="xmlStr">xml</param>
        /// <param name="rootName">给定的节点</param>
        /// <returns>指定的类型</returns>
        public static T DeserializeObj<T>(string xmlStr,string rootName) where T : class
        {
            if (string.IsNullOrEmpty(xmlStr))
                return null;
            T item = Activator.CreateInstance<T>();
            var xmlDoc = new XmlDocument();
            xmlDoc.LoadXml(xmlStr);
            foreach (XmlNode itemNode in xmlDoc.GetElementsByTagName(rootName).Item(0).ChildNodes)
            {
                PropertyInfo[] properties = item.GetType().GetProperties();
                foreach (XmlNode propertyNode in itemNode.ChildNodes)
                {
                    string name = propertyNode.Name;

                    string value = propertyNode.InnerText;
                    foreach (PropertyInfo property in properties)
                    {
                        if (name == property.Name)
                        {
                            if (property.PropertyType == typeof(string))
                                property.SetValue(item, Convert.ChangeType(value, property.PropertyType), null);

                        }
                    }

                }
            }
            return item;
        }


        /// <summary>
        /// by make:2012/6/28 xfy
        /// 通过反射解析xml内容并填充到对象中,这里通过递归实现，可以解析所有嵌套
        /// 注意:1.xml的内容与类型的嵌套需要保持一致
        ///      2.类型的属性类和xml标签需要一样
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="xmlStr"></param>
        /// <param name="rootName"></param>
        /// <returns></returns>
        public static List<T> Deserialize<T>(string xmlStr, string rootName)
        {
            var result = new List<T>();
            var xmlDoc = new XmlDocument();
            xmlDoc.LoadXml(xmlStr);
            try
            {
                foreach (XmlNode itemNode in xmlDoc.GetElementsByTagName(rootName).Item(0).ChildNodes)
                {
                    T item = Activator.CreateInstance<T>();
                    PropertyInfo[] properties = item.GetType().GetProperties();
                    foreach (XmlNode propertyNode in itemNode.ChildNodes)
                    {
                        string name = propertyNode.Name;
                        string value = propertyNode.InnerText;
                        foreach (PropertyInfo property in properties)
                        {
                            if (name == property.Name)
                            {
                                if (property.PropertyType == typeof(string))
                                    property.SetValue(item, Convert.ChangeType(value, property.PropertyType), null);
                                else
                                {
                                    var subXmlDoc = new XmlDocument();
                                    subXmlDoc.LoadXml(propertyNode.OuterXml);

                                    PaselXml(item, property, subXmlDoc);
                                }
                                break;
                            }//if
                        }//propertyInfo
                    }//XmlNode
                    result.Add(item);
                }//for
            }
            catch (Exception ex)
            {
            }
            return result;
        }

        public static void PaselXml(object item, PropertyInfo property, XmlDocument xmlDoc)
        {
            try
            {
                object subItem = Activator.CreateInstance(property.PropertyType);
                PropertyInfo[] subPros = property.PropertyType.GetProperties();
                if (subPros != null)
                {
                    foreach (XmlNode trdpropertyNode in xmlDoc.GetElementsByTagName(property.Name).Item(0).ChildNodes)
                    {
                        string subName = trdpropertyNode.Name;
                        string subValue = trdpropertyNode.InnerText;
                        foreach (PropertyInfo subproperty in subPros)
                        {
                            if (subproperty.Name == subName)
                            {
                                if (subproperty.PropertyType == typeof(string))
                                    subproperty.SetValue(subItem, Convert.ChangeType(subValue, subproperty.PropertyType), null);
                                else
                                {
                                    var subXmlDoc = new XmlDocument();
                                    subXmlDoc.LoadXml(trdpropertyNode.OuterXml);

                                    PaselXml(subItem, subproperty, subXmlDoc);
                                }
                                break;
                            }
                        }
                    }
                    string s = subItem.GetType().ToString();
                    property.SetValue(item, Convert.ChangeType(subItem, property.PropertyType), null);
                }
            }
            catch (Exception ex)
            {
            }
        }   

       

        /// <summary>
        /// 使用序列化的方式，深克隆一个对象。
        /// </summary>
        public static object CloneObject(object obj)
        {
            // 创建内存流
            using (var ms = new MemoryStream())
            {
                // 创建序列化器（有的书称为串行器）
                // 创建一个新的序列化器对象总是比较慢。
                var bf = new BinaryFormatter(null, new StreamingContext(StreamingContextStates.Clone));

                // 将对象序列化至流
                bf.Serialize(ms, obj);

                // 将流指针指向第一个字符
                ms.Seek(0, SeekOrigin.Begin);

                // 反序列化至另一个对象（即创建了一个原对象的深表副本）
                object cloneObject = bf.Deserialize(ms);

                // 关闭流 
                ms.Close();
                return cloneObject;
            }
        }

        public static string Serialize<T>(List<T> genericList, string rootName, string eleName, string attributeName)
        {
            var result = new XmlDocument();
            result.LoadXml("<" + rootName + "></" + rootName + ">");
            foreach (T obj in genericList)
            {
                XmlElement item = result.CreateElement(eleName);

                PropertyInfo[] properties = obj.GetType().GetProperties();
                foreach (PropertyInfo property in properties)
                {
                    if (property.GetValue(obj, null) != null)
                    {
                        XmlElement element = result.CreateElement(property.Name);

                        element.SetAttribute(attributeName, property.PropertyType.Name);
                        element.InnerText = property.GetValue(obj, null).ToString();
                        item.AppendChild(element);
                    }
                }
                if (result.DocumentElement != null) result.DocumentElement.AppendChild(item);
            }
            return result.InnerXml;
        }

        public static string Serialize<T>(List<T> genericList, string rootName)
        {
            var result = new XmlDocument();
            result.LoadXml("<" + rootName + "></" + rootName + ">");
            foreach (T obj in genericList)
            {
                PropertyInfo[] properties = obj.GetType().GetProperties();
                foreach (PropertyInfo property in properties)
                {
                    if (property.GetValue(obj, null) != null)
                    {
                       XmlElement element = result.CreateElement(property.Name);
                       element.InnerText = property.GetValue(obj, null).ToString();
                       if (result.DocumentElement != null) 
                           result.DocumentElement.AppendChild(element);
                    }
                }
               }
            return result.InnerXml;
        }

        public static string Serialize<T>(T obj, string rootName)
        {
            var result = new XmlDocument();
            result.LoadXml("<" + rootName + "></" + rootName + ">");
                PropertyInfo[] properties = obj.GetType().GetProperties();
                foreach (PropertyInfo property in properties)
                {
                    if (property.GetValue(obj, null) != null)
                    {
                        XmlElement element = result.CreateElement(property.Name);


                        element.InnerText = property.GetValue(obj, null).ToString();


                        if (result.DocumentElement != null) result.DocumentElement.AppendChild(element);
                    }
                }
                return result.InnerXml;
        }
        
   }
}